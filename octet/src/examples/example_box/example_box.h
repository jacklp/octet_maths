#include "tinyxml/tinyxml.h"
#include <typeinfo>
#include <string>
#include <iostream>
#include <sstream>

namespace octet {
	/// Scene containing a box with octet.
	class example_box : public app {
		// scene for drawing box
		ref<visual_scene> app_scene;
		vec3 currentPoint = vec3(0);
		float currentAngle = 0.0f;

		material *black;

		//All the variables I need
		std::map<char, std::string> rules;
		float length;
		float width;
		float rotation;
		int current_increment_count;
		int max_increment_count;
		std::string current_rule_set;
		std::string axiom;
		std::vector<std::pair<vec3, float>> stack;
		float sceneHeightY;
		std::string acceptableOperators = "[]-+";
		int numBranches = 0;
		float cameraZoomRatio = 0.0f;

	public:

		/// this is called when we construct the class before everything is initialised.
		example_box(int argc, char **argv) : app(argc, argv) {
		}

		/// this is called once OpenGL is initialized
		void app_init() {
			
			
			black = new material(vec4(0, 0, 0, 1));
			
			//on load of new config repeat this:
			reset_defaults();
			load_xml();

			parse();
			parse();
			parse();
			parse();
			parse();
			parse();
			parse();
			render();
			
			//on key press run the next increment
			/*if (current_increment_count != max_increment_count){

				//increment and update rule set
				parse();

				//increase the increment count by 1
				current_increment_count++;
			}*/

		}

		void reset_defaults(){
			app_scene = new visual_scene();
			app_scene->reset();
			app_scene->create_default_camera_and_lights();
			app_scene->get_camera_instance(0)->set_far_plane(100000000.0f);
			rules.clear();
			length = 1.0f;
			width = 1.0f;
			rotation = 90.0f;
			current_increment_count = 0;
			max_increment_count = 5;
			numBranches = 0;
		}

		void render(){
			//loop through the current rule set

			for (int i = 0; i < current_rule_set.size(); ++i){
				char test = current_rule_set[i];
				switch (test){
				case 'F':
					{
						float halfSize = 1.0f;
						mat4t mat = mat4t();
						mat.loadIdentity();
						mat.rotate(90.0f, 1, 0, 0);

						//calculate curent section.
						vec3 midPoint = currentPoint;
						midPoint.x() = midPoint.x() + halfSize *cos((currentAngle + 90) * CL_M_PI / 180);
						midPoint.y() = midPoint.y() + halfSize *sin((currentAngle + 90)* CL_M_PI / 180);

						//calculate next section to be saved
						vec3 endPoint = currentPoint;
						endPoint.x() = endPoint.x() + 2.0f*halfSize *cos((currentAngle + 90) * CL_M_PI / 180);
						endPoint.y() = endPoint.y() + 2.0f*halfSize *sin((currentAngle + 90) * CL_M_PI / 180);

						//instantiate object and add to the scene
						mesh_box *line = new mesh_box(vec3(0.2f, 0.1f, halfSize), mat);
						scene_node *node = new scene_node();
						app_scene->add_child(node);
						app_scene->add_mesh_instance(new mesh_instance(node, line, black));
						node->translate(midPoint);
						node->rotate(currentAngle, vec3(0, 0, 1));
						
						if (endPoint.y() > sceneHeightY){
							sceneHeightY = endPoint.y();
							numBranches++;
						}
						
						currentPoint = endPoint;
						break;
					}
					case '[':
						stack.push_back(std::pair<vec3, float>(currentPoint, currentAngle));
						break;
					case ']':
						currentPoint = stack[stack.size() - 1].first;
						currentAngle = stack[stack.size() - 1].second;
						stack.pop_back();
						stack.shrink_to_fit();
						break;
					case '+':
						currentAngle += rotation;
						break;
					case '-':
						currentAngle -= rotation;
						break;
					case 'X':
						break;
				}
			}
		}

		void parse(){

			std::string current_rule_set_temp = "";
			for (int count = 0; count <= current_rule_set.size(); ++count){
				boolean match = false;
				typedef std::map<char, std::string>::iterator it_type;
				for (it_type rule_it = rules.begin(); rule_it != rules.end(); ++rule_it){
					if (rule_it->first == current_rule_set[count] && match != true){
						current_rule_set_temp += rule_it->second;
						match = true;
					}
					else{
						for (int i = 0; i <= acceptableOperators.size(); ++i){
							if (current_rule_set[count] == acceptableOperators[i] && match != true){
								current_rule_set_temp += current_rule_set[count];
								match = true;
							}
						}
					}
				}
			}
			current_rule_set = current_rule_set_temp;
		}

		void load_xml() {

			TiXmlDocument doc("xml/config1.xml");
			doc.LoadFile();

			TiXmlElement *parameters, *parameter;

			parameters = doc.FirstChildElement("parameters");

			if (parameters)
			{

				parameter = parameters->FirstChildElement("parameter");

				while (parameter)
				{
					string type = string(parameter->Attribute("type"), sizeof(parameter->Attribute("type")));

					if (type == "length"){
						std::string text = string(parameter->GetText(), sizeof(parameter->GetText()));
						std::string::size_type sz;
						length = std::stof(text, &sz);
					}
					else if (type == "width"){
						std::string text = string(parameter->GetText(), sizeof(parameter->GetText()));
						std::string::size_type sz;
						width = std::stof(text, &sz);
					}
					else if (type == "angle"){
						std::string text = string(parameter->GetText(), sizeof(parameter->GetText()));
						std::string::size_type sz;
						rotation = std::stof(text, &sz);
					}
					else if (type == "rule"){

						unsigned int intValue;
						std::stringstream s(parameter->Attribute("size"));
						s >> intValue;

						std::string text = string(parameter->GetText(), intValue);

						char pre = text.substr(0, text.find_first_of("->"))[0];
						std::string post = text.substr(text.find_first_of("->") + 2, text.size());
						rules[pre] = post;
					}
					else if (type == "axiom"){
						current_rule_set = std::string(parameter->GetText(), sizeof(parameter->GetText()));
					}
					else if (type == "inc"){
						std::stringstream s(parameter->GetText());
						s >> max_increment_count;
					}

					parameter = parameter->NextSiblingElement("parameter");
				}
			}
		}
		
		/// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {
			int vx = 0, vy = 0;
			get_viewport_size(vx, vy);
			app_scene->begin_render(vx, vy);

			// update matrices. assume 30 fps.
			app_scene->update(1.0f / 30);

			// draw the scene
			app_scene->render((float)vx / vy);

			//app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, sceneHeightY / 2, 1.0f * numBranches));

			vec3 cameraPos = app_scene->get_camera_instance(0)->get_node()->get_position();
		}
	};
}
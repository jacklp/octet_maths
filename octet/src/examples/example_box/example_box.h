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

		std::map<char, std::string> rules;
		float length = 0.5f;
		float width = 1.0f;
		float rotation = 15.0f;
		float current_increment_count = 0.0f;
		int max_increment_count = 5;
		std::string axiom;
		std::vector<std::pair<vec3, float>> stack;
		float sceneHeightY = 0.0f;
		std::string acceptableOperators = "[]-+";
		int numBranches = 0;
		float cameraZoomRatio = 0.0f;
		string docNum = "1";
		std::vector<std::string> increment_vector;

	public:

		/// this is called when we construct the class before everything is initialised.
		example_box(int argc, char **argv) : app(argc, argv) {
		}

		/// this is called once OpenGL is initialized
		void app_init() {
			
			black = new material(vec4(0, 0, 0, 1));
			reset();
			load_xml();
			parse();
		}

		void reset(){
			app_scene = new visual_scene();
			app_scene->create_default_camera_and_lights();
			app_scene->get_camera_instance(0)->get_node()->translate(vec3(0.0f, 0.0f, 2.0f));
			app_scene->get_camera_instance(0)->set_far_plane(100000.0f);
			currentPoint = vec3(0);
			currentAngle = 0.0f;
			sceneHeightY = 0.0f;
		}

		void render(){
			//loop through the current rule set

			for (int i = 0; i < increment_vector[current_increment_count].size(); ++i){
				char test = increment_vector[current_increment_count][i];
				switch (test){
				case 'F':
					{
						
						//calculate curent section.
						vec3 midPoint = currentPoint;
						midPoint.x() = currentPoint.x() + length *cos((currentAngle + 90) * CL_M_PI / 180);
						midPoint.y() = currentPoint.y() + length *sin((currentAngle + 90)* CL_M_PI / 180);

						//calculate next section to be saved
						vec3 endPoint = currentPoint;
						endPoint.x() = currentPoint.x() + 2.0f*length *cos((currentAngle + 90) * CL_M_PI / 180);
						endPoint.y() = currentPoint.y() + 2.0f*length *sin((currentAngle + 90) * CL_M_PI / 180);

						if (endPoint.y() > sceneHeightY){
							sceneHeightY = endPoint.y();
						}

						//configure matrix
						mat4t mat = mat4t();
						mat.loadIdentity();
						mat.translate(midPoint);
						mat.rotate(currentAngle, 0.0f, 0.0f, 1.0f);

						//configure box
						mesh_box *box = new mesh_box(vec3(width, length, width), mat);
						scene_node *node = new scene_node();
						app_scene->add_child(node);
						app_scene->add_mesh_instance(new mesh_instance(node, box, black));
			
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

			//get everything in to a increment vector
			for (int increment = 1; increment <= max_increment_count; increment++){
				std::string current_rule_set_temp = "";
				for (int count = 0; count <= increment_vector[increment-1].size(); ++count){
					boolean match = false;
					typedef std::map<char, std::string>::iterator it_type;
					for (it_type rule_it = rules.begin(); rule_it != rules.end(); ++rule_it){
						if (rule_it->first == increment_vector[increment-1][count] && match != true){
							current_rule_set_temp += rule_it->second;
							match = true;
						}
						else{
							for (int i = 0; i <= acceptableOperators.size(); ++i){
								if (increment_vector[increment-1][count] == acceptableOperators[i] && match != true){
									current_rule_set_temp += increment_vector[increment - 1][count];
									match = true;
								}
							}
						}
					}
				}
				increment_vector.push_back(std::string(current_rule_set_temp));
			}
		}

		string get_path(){

			string result = "xml/config";
			result.insert(10, docNum);
			result.truncate(11);
			string post = ".xml";
			result.insert(11, post);
			result.truncate(15);
			
			return result;
		}

		void load_xml() {

			rules.clear();
			increment_vector.clear();
			string path = get_path();
			TiXmlDocument doc(path);
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
						increment_vector.push_back(std::string(parameter->GetText(), sizeof(parameter->GetText())));
					}
					else if (type == "inc"){
						std::stringstream s(parameter->GetText());
						s >> max_increment_count;
					}

					parameter = parameter->NextSiblingElement("parameter");
				}
			}
		}
		
		void input(){

			if (is_key_going_down(key_up)) {
				if (current_increment_count < max_increment_count && current_increment_count >= 0.0f){
					current_increment_count++;
					reset();
					render();
					camera();
				}
			}
			if (is_key_going_down(key_down)){
				if (current_increment_count <= max_increment_count && current_increment_count > 0.0f){
					current_increment_count--;
					reset();
					render();
					camera();
				}
				//need to make parse work backwards.
			}
			if (is_key_going_down(key_right)){
				if (docNum[0] != '9'){
					docNum[0]++;
					current_increment_count = 0.0f;
					load_xml();
					parse();
				}
			}
			if (is_key_going_down(key_left)){
				if (docNum[0] != '1'){
					docNum[0]--;
					current_increment_count = 0.0f;
					load_xml();
					parse();
				}
			}
		}

		void camera(){

			float padding = pow(current_increment_count, 2.5);
			app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, sceneHeightY / 2, sceneHeightY + padding));
			
		}

		/// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {

			input();

			app_scene->begin_render(w, h);

			// update matrices. assume 30 fps.
			app_scene->update(1.0f / 30.0f);

			// draw the scene
			app_scene->render((float)w / h);

			
		}
	};
}
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

	public:

		/// this is called when we construct the class before everything is initialised.
		example_box(int argc, char **argv) : app(argc, argv) {
		}

		/// this is called once OpenGL is initialized
		void app_init() {
			app_scene = new visual_scene();
			app_scene->create_default_camera_and_lights();
			
			black = new material(vec4(0, 0, 0, 1));
			
			//on load of new config repeat this:
			reset_defaults();
			load_assets_via_xml();


			//on key press run the next increment
			render_lines();

		}

		void reset_defaults(){
			rules.clear();
			length = 1.0f;
			width = 1.0f;
			rotation = 90.0f;
			current_increment_count = 0;
			max_increment_count = 5;
		}

		void render_lines(){

			// check if you have not reached the end of the increments configured.
			if (current_increment_count != max_increment_count){
				
				// draw current rule set
				draw_current_rule_set(currentPoint);
				
				//increment and update rule set
				update_rule_set();
				update_rule_set();
				update_rule_set();
				update_rule_set();
				update_rule_set();
				update_rule_set();
				update_rule_set();

				//increase the increment count by 1
				current_increment_count++;
			}
		}

		void draw_current_rule_set(vec3 startingPoint)
		{

			//loop through the rules and 
			float halfSize = -1.0f;

			vec3 midPoint = startingPoint;
			midPoint.z() = midPoint.z() + halfSize;

			vec3 endPoint = startingPoint;
			endPoint.z() = endPoint.z() + 2.0f*halfSize;
			
			mat4t mat = mat4t();
			mat.loadIdentity();
			mat.rotate(90.0f, 1, 0, 0);
			
			mesh_cylinder *line = new mesh_cylinder(zcylinder(midPoint, 0.1f, halfSize), mat);

			scene_node *node = new scene_node();
			app_scene->add_child(node);
			app_scene->add_mesh_instance(new mesh_instance(node, line, black));

		}

		void update_rule_set(){

			//loop through all of the rules and apply them to current_rule_set
			typedef std::map<char, std::string>::iterator it_type;
			for (it_type rule_it = rules.begin(); rule_it != rules.end(); ++rule_it){

				//loop through a rule inside of the rules map and apply it to curent_rule_set
				char needle = rule_it->first;
				std::string rule = rule_it->second;
				std::string::iterator str_it;

				std::string current_rule_set_temp = current_rule_set;

				for (int count = 0; count < current_rule_set.size(); ++count){
					char haystack = current_rule_set[count];

					//if key matches a value in our current rule set then replace it with the corresponding rule content
					if (needle == haystack){

						//remove the key and replace with rule
						current_rule_set_temp.erase(count, 1);
						current_rule_set_temp.insert(count, rule);
					}
				}
				current_rule_set = current_rule_set_temp;
			}
			printf("%s", current_rule_set);
		}

		TiXmlDocument load_xml() {
			TiXmlDocument doc("xml/config.xml");
			GLboolean loadOkay = doc.LoadFile();

			if (loadOkay) {
				return doc;
			}
			else {
				return false;
			}

		}

		void load_assets_via_xml() {

			TiXmlDocument doc = load_xml();
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
					else if (type == "rotation"){
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

		}
	};
}
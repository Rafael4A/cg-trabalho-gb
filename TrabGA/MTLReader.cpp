#include "MTLReader.h"
#include <vector>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>     
#include <fstream> 
#include <sstream>
#include <bitset>

using std::vector;
using glm::vec2;
using glm::vec3;
using namespace std;

vector<Material*> MTLReader::readFile(string pathfile)
{
	vector <Material*> materials;
	Material* currentMaterial = new Material;

	bool firstMaterial = true;
	ifstream file(pathfile);

	while (!file.eof()) {
		string line;
		getline(file, line);

		stringstream sline;
		sline << line;

		string temp;
		sline >> temp;
		if (temp == "newmtl") {
			string name;
			sline >> name;
			if (firstMaterial) {
				materials.push_back(currentMaterial);
				currentMaterial->id = name;
				firstMaterial = false;
			}
			else {
				Material* newMaterial = new Material;
				newMaterial->id = name;
				currentMaterial = newMaterial;
				materials.push_back(currentMaterial);
			}

		}
		else if (temp == "Ns") {
			float ns;
			sline >> ns;
			currentMaterial->Ns = ns;
		}
		else if (temp == "Ka") {
			float x, y, z;
			sline >> x >> y >> z;
			glm::vec3 ka = glm::vec3(x, y, z);
			currentMaterial->ka = ka;
		}
		else if (temp == "Kd") {
			float x, y, z;
			sline >> x >> y >> z;
			glm::vec3 kd = glm::vec3(x, y, z);
			currentMaterial->kd = kd;
		}
		else if (temp == "Ks") {
			float x, y, z;
			sline >> x >> y >> z;
			glm::vec3 ks = glm::vec3(x, y, z);
			currentMaterial->ks = ks;
		}
		else if (temp == "map_Kd") {
			string mapkd;
			sline >> mapkd;
			currentMaterial->map_kd = mapkd;
		}
	}

	return materials;
}

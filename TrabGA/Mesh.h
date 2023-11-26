#pragma once
#include "Group.h"
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
#include "Material.h"

using std::vector;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using namespace std;

class Mesh {
public:

	vec4 min;
	vec4 max;
	string mtllib;
	vector <Group*> groups;
	vector <vec3*> vertex;
	vector <vec3*> normals;
	vector <vec2*> mappings;
	vector <Material*> materials;

};
#pragma once
#include "Face.h"
#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include "Material.h"

using std::string;
using std::vector;

class Group {
public:
    string name;
    string materialName;
    vector <Face*> faces;
    GLuint vao;
    Material* mat;
};
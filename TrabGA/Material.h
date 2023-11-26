#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <string>

using std::vector;
using std::string;
using glm::vec3;

class Material {
public:
    string id;
    vec3 ka, kd, ks;
    float Ns;
    string map_kd;
    GLuint TID_mapKd;
    unsigned int TID_mapKs;
};

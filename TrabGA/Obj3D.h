#pragma once

#include "Mesh.h"
#include <glm/glm.hpp>

class Obj3D {
public:
    string filePath;
    string name;
    Mesh* mesh;
    glm::mat4 transform;
    int status;
    bool eliminavel;
    glm::vec3 direcao;
};
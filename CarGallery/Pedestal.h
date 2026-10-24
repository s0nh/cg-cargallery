#pragma once

#include "TriMesh.h"

// Round display stand: a cylinder standing on the floor (y = 0) at the given x/z
class Pedestal
{
public:
    glm::vec3 color = glm::vec3(0.75f, 0.75f, 0.78f);

    Pedestal(glm::vec3 center, float radius, float height, GLuint program);

    // where a car should stand: center of the top face
    glm::vec3 top() const { return center + glm::vec3(0, height, 0); }

    void draw(GLuint program) const;

private:
    glm::vec3 center;
    float height;
    TriMesh mesh;
};

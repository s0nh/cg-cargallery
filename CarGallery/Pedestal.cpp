#include "Pedestal.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const int SEGMENTS = 64;  // more segments = rounder circle

Pedestal::Pedestal(glm::vec3 center, float radius, float height, GLuint program)
    : center(center), height(height)
{
    // built around the origin; draw() moves it to center
    for (int i = 0; i < SEGMENTS; i++) {
        float a0 = glm::radians(360.0f * i / SEGMENTS);
        float a1 = glm::radians(360.0f * (i + 1) / SEGMENTS);
        glm::vec3 d0(cos(a0), 0, -sin(a0));  // counter-clockwise when seen from above
        glm::vec3 d1(cos(a1), 0, -sin(a1));
        glm::vec3 up(0, height, 0);

        // side: one quad = 2 triangles, normals point outward (smooth: per-vertex direction)
        glm::vec3 side[6] = { d0 * radius, d1 * radius, d1 * radius + up, d0 * radius, d1 * radius + up, d0 * radius + up };
        glm::vec3 sideN[6] = { d0, d1, d1, d0, d1, d0 };
        for (int k = 0; k < 6; k++) {
            mesh.positions.push_back(side[k]);
            mesh.normals.push_back(sideN[k]);
        }

        // top cap: a pie slice from the center; the bottom sits on the floor and is never seen
        glm::vec3 cap[3] = { up, d0 * radius + up, d1 * radius + up };
        for (int k = 0; k < 3; k++) {
            mesh.positions.push_back(cap[k]);
            mesh.normals.push_back(glm::vec3(0, 1, 0));
        }
    }
    mesh.upload(program);
}

void Pedestal::draw(GLuint program) const
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(glGetUniformLocation(program, "objectColor"), 1, glm::value_ptr(color));
    mesh.draw();
}

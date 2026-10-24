#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

// Closed exhibition hall: a box seen from the inside.
// Floor sits on y = 0, centered on the origin in x/z.
class Room
{
public:
    glm::vec3 floorColor = glm::vec3(0.35f, 0.35f, 0.38f);
    glm::vec3 wallColor = glm::vec3(0.85f, 0.85f, 0.82f);
    glm::vec3 ceilingColor = glm::vec3(0.6f, 0.6f, 0.6f);

    // needs a current GL context and the program whose vPosition/vNormal it feeds
    Room(float width, float height, float depth, GLuint program);

    void draw(GLuint program) const;

private:
    static const int NumVertices = 36;  // 6 faces * 2 triangles * 3 vertices
    glm::vec4 points[NumVertices];
    glm::vec3 normals[NumVertices];
    int index = 0;
    GLuint vao;

    // a, b, c, d counter-clockwise when seen from inside the room
    void quad(glm::vec4 a, glm::vec4 b, glm::vec4 c, glm::vec4 d, glm::vec3 n);
};

#include "Room.h"
#include <glm/gtc/type_ptr.hpp>

#define BUFFER_OFFSET(x)  ((const void*) (x))

Room::Room(float width, float height, float depth, GLuint program)
{
    float w = width / 2, h = height, d = depth / 2;

    // normals point INTO the room; vertex order is CCW seen from inside, so back-face culling keeps them
    // draw() relies on this order: floor, ceiling, then the 4 walls
    quad(glm::vec4(-w, 0, -d, 1), glm::vec4(-w, 0, d, 1), glm::vec4(w, 0, d, 1), glm::vec4(w, 0, -d, 1), glm::vec3(0, 1, 0));   // floor
    quad(glm::vec4(-w, h, -d, 1), glm::vec4(w, h, -d, 1), glm::vec4(w, h, d, 1), glm::vec4(-w, h, d, 1), glm::vec3(0, -1, 0));  // ceiling
    quad(glm::vec4(-w, 0, -d, 1), glm::vec4(w, 0, -d, 1), glm::vec4(w, h, -d, 1), glm::vec4(-w, h, -d, 1), glm::vec3(0, 0, 1));  // back  (z = -d)
    quad(glm::vec4(w, 0, d, 1), glm::vec4(-w, 0, d, 1), glm::vec4(-w, h, d, 1), glm::vec4(w, h, d, 1), glm::vec3(0, 0, -1));     // front (z = +d)
    quad(glm::vec4(-w, 0, d, 1), glm::vec4(-w, 0, -d, 1), glm::vec4(-w, h, -d, 1), glm::vec4(-w, h, d, 1), glm::vec3(1, 0, 0));  // left  (x = -w)
    quad(glm::vec4(w, 0, -d, 1), glm::vec4(w, 0, d, 1), glm::vec4(w, h, d, 1), glm::vec4(w, h, -d, 1), glm::vec3(-1, 0, 0));     // right (x = +w)

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals);

    GLuint vPos = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPos);
    glVertexAttribPointer(vPos, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vNorm = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNorm);
    glVertexAttribPointer(vNorm, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

    glBindVertexArray(0);
}

void Room::quad(glm::vec4 a, glm::vec4 b, glm::vec4 c, glm::vec4 d, glm::vec3 n)
{
    glm::vec4 order[6] = { a, b, c, a, c, d };
    for (int i = 0; i < 6; i++) {
        points[index] = order[i];
        normals[index] = n;
        index++;
    }
}

void Room::draw(GLuint program) const
{
    GLint color = glGetUniformLocation(program, "objectColor");
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

    glBindVertexArray(vao);
    glUniform3fv(color, 1, glm::value_ptr(floorColor));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUniform3fv(color, 1, glm::value_ptr(ceilingColor));
    glDrawArrays(GL_TRIANGLES, 6, 6);
    glUniform3fv(color, 1, glm::value_ptr(wallColor));
    glDrawArrays(GL_TRIANGLES, 12, 24);
    glBindVertexArray(0);
}

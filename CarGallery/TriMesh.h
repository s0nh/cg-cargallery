#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// Triangle list (every 3 positions = 1 triangle), drawn with the same
// vPosition/vNormal attributes as Room
class TriMesh
{
public:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;

    // needs a current GL context and the program whose vPosition/vNormal it feeds
    void upload(GLuint program);
    void draw() const;

private:
    GLuint vao = 0;
};

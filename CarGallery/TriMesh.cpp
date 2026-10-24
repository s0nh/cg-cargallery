#include "TriMesh.h"

#define BUFFER_OFFSET(x)  ((const void*) (x))

void TriMesh::upload(GLuint program)
{
    size_t posBytes = positions.size() * sizeof(glm::vec3);
    size_t normBytes = normals.size() * sizeof(glm::vec3);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, posBytes + normBytes, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, posBytes, positions.data());
    glBufferSubData(GL_ARRAY_BUFFER, posBytes, normBytes, normals.data());

    // 3 floats per position; the shader's vec4 vPosition gets w = 1 automatically
    GLuint vPos = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPos);
    glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vNorm = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNorm);
    glVertexAttribPointer(vNorm, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(posBytes));

    glBindVertexArray(0);
}

void TriMesh::draw() const
{
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)positions.size());
    glBindVertexArray(0);
}

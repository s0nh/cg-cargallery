#include "Car.h"
#include <algorithm>
#include <cctype>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Car::Car(const std::string& name, const std::string& objPath, const std::string& paintMaterial,
         glm::vec3 standOn, float length, GLuint program)
    : name(name), paintMaterial(paintMaterial)
{
    if (!obj.load(objPath)) return;
    for (ObjPart& part : obj.parts)
        if (!part.mesh->positions.empty()) part.mesh->upload(program);

    // downloaded models come in any unit and origin: scale the longer side (x or z) to `length`
    // meters, and put the bottom center of the bounding box on standOn.
    // read right to left: center at origin -> scale -> move onto the stand
    glm::vec3 size = obj.bboxMax - obj.bboxMin;
    glm::vec3 bottomCenter((obj.bboxMin.x + obj.bboxMax.x) / 2, obj.bboxMin.y, (obj.bboxMin.z + obj.bboxMax.z) / 2);
    float scale = length / glm::max(size.x, size.z);
    model = glm::translate(glm::mat4(1.0f), standOn)
          * glm::scale(glm::mat4(1.0f), glm::vec3(scale))
          * glm::translate(glm::mat4(1.0f), -bottomCenter);
}

// the material name only tells what a part is; its color is our choice
glm::vec3 Car::partColor(const std::string& material) const
{
    if (material == paintMaterial) return config.bodyColor;

    std::string m = material;
    std::transform(m.begin(), m.end(), m.begin(), ::tolower);
    auto has = [&m](const char* word) { return m.find(word) != std::string::npos; };

    if (has("glass") || has("window")) return glm::vec3(0.10f, 0.12f, 0.15f);            // tinted glass (opaque for now)
    if (has("tire") || has("thread") || has("sidewall")) return glm::vec3(0.05f);         // rubber
    if (has("chrome") || has("rim") || has("metal")) return glm::vec3(0.70f, 0.70f, 0.72f);
    if (has("light")) return glm::vec3(0.95f, 0.95f, 0.90f);
    return glm::vec3(0.15f);  // everything else: dark trim
}

void Car::draw(GLuint program) const
{
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    GLint colorLoc = glGetUniformLocation(program, "objectColor");

    // downloaded models often have faces wound the wrong way; don't let culling punch holes in them
    glDisable(GL_CULL_FACE);
    for (const ObjPart& part : obj.parts) {
        if (part.mesh->positions.empty()) continue;  // e.g. the "default" part when every face has a usemtl
        glUniform3fv(colorLoc, 1, glm::value_ptr(partColor(part.material)));
        part.mesh->draw();
    }
    glEnable(GL_CULL_FACE);
}

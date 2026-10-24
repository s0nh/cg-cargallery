#pragma once

#include "ObjLoader.h"
#include <glm/glm.hpp>
#include <string>

// What the user can change (more fields later: finish, wheel color, spoke count, ...)
struct CarConfig
{
    glm::vec3 bodyColor = glm::vec3(0.8f, 0.1f, 0.1f);
};

// One car model standing on a spot in the hall.
// The OBJ gives only the shape; every color is decided here, by what each part is.
class Car
{
public:
    std::string name;
    CarConfig config;

    // paintMaterial: name of the OBJ material ("usemtl") that is the body paint
    // yawDegrees: turn around the vertical axis (0 = front faces +z)
    // length: real length of the car in meters; the model is scaled to it
    Car(const std::string& name, const std::string& objPath, const std::string& paintMaterial,
        glm::vec3 standOn, float yawDegrees, float length, GLuint program);

    bool loaded() const { return !obj.parts.empty(); }
    void draw(GLuint program) const;

private:
    ObjLoader obj;
    std::string paintMaterial;
    glm::mat4 model = glm::mat4(1.0f);

    glm::vec3 partColor(const std::string& material) const;
};

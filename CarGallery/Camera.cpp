#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 startPos, float startYaw, float startPitch)
    : position(startPos), yaw(startYaw), pitch(startPitch)
{
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, position + front, worldUp);
}

void Camera::move(float forward, float right, float dt)
{
    // walk on the floor: drop the vertical part so looking up doesn't make us fly
    glm::vec3 walkFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    glm::vec3 dir = walkFront * forward + rightDir * right;

    // W+D must not be faster than W alone
    if (glm::length(dir) > 0.0f)
        position += glm::normalize(dir) * moveSpeed * dt;
}

void Camera::rotate(float dx, float dy)
{
    yaw += dx * sensitivity;
    pitch += dy * sensitivity;

    // at 90 degrees front is parallel to worldUp and cross() becomes zero
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    updateVectors();
}

void Camera::updateVectors()
{
    float y = glm::radians(yaw);
    float p = glm::radians(pitch);
    front = glm::normalize(glm::vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
    rightDir = glm::normalize(glm::cross(front, worldUp));
}

#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
    glm::vec3 position;
    float yaw;    // 좌우 시선 각도 -90 -Z방향
    float pitch;  // 상하 시선 각도  [-89, 89]

    float moveSpeed = 3.0f;     // 이동속도
    float sensitivity = 0.1f;   // 마우스 감도

    Camera(glm::vec3 startPos, float startYaw = -90.0f, float startPitch = 0.0f);

    glm::mat4 getViewMatrix() const;

    // forward/right: -1, 0, or 1 from the keys (W=+forward, S=-forward, D=+right, A=-right)
    void move(float forward, float right, float dt);

    // dx, dy: mouse movement in pixels since the last call (dy > 0 means mouse moved up)
    void rotate(float dx, float dy);

private:
    glm::vec3 front;
    glm::vec3 rightDir;
    const glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // recompute front/rightDir from yaw/pitch
    void updateVectors();
};

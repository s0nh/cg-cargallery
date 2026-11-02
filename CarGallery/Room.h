#pragma once

#include "TriMesh.h"
#include <vector>

// A display bay carved into the hall wall: where a pedestal and its car go
struct Bay
{
    glm::vec3 center;  // on the floor, center of the bay's round end: where the pedestal goes
    glm::vec3 inward;  // unit vector from the bay toward the hall center
};

// Round exhibition hall seen from the inside: a circular wall with display bays
// pushed out of it. Seen from above each bay is a U (straight sides, round end).
// Only the walls curve: floor (y = 0) and ceiling are flat. Centered on the origin in x/z.
class Room
{
public:
    glm::vec3 floorColor = glm::vec3(0.35f, 0.35f, 0.38f);
    glm::vec3 wallColor = glm::vec3(0.85f, 0.85f, 0.82f);
    glm::vec3 bayColor = glm::vec3(0.70f, 0.72f, 0.78f);
    glm::vec3 ceilingColor = glm::vec3(0.6f, 0.6f, 0.6f);

    // radius: hall radius, height: floor to ceiling
    // bayAngles: direction of each bay in degrees, measured from +x toward +z (-90 = straight back, -z)
    // needs a current GL context and the program whose vPosition/vNormal it feeds
    Room(float radius, float height, const std::vector<float>& bayAngles, GLuint program);

    const std::vector<Bay>& bays() const { return bayList; }

    // Keep a walker (a circle of bodyRadius on the floor plan) off the walls:
    // returns pos moved back to the nearest place it fits. Only x/z change.
    glm::vec3 collide(glm::vec3 pos, float bodyRadius) const;

    void draw(GLuint program) const;

private:
    float hallRadius;
    std::vector<Bay> bayList;
    TriMesh floor, ceiling, wall, bayWall;

    void flatTri(glm::vec3 a, glm::vec3 b, glm::vec3 c, float height);
};

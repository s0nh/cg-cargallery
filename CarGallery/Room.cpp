#include "Room.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

static const float PI = 3.14159265f;
static const int SEGMENTS = 128;       // hall circle subdivision for a full turn
static const float BAY_RADIUS = 4.2f;  // half the width of a bay
static const float BAY_DEPTH = 3.0f;   // straight part of a bay's side walls, before its round end

static const glm::vec3 UP(0, 1, 0);

// horizontal unit vector at angle a (radians), measured from +x toward +z
static glm::vec3 dir(float a) { return glm::vec3(std::cos(a), 0, std::sin(a)); }

// One triangle. The vertex order is flipped when needed so the front face is on the side
// the normals point to: callers only give the right normals, culling then keeps the face.
static void tri(TriMesh& m, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 na, glm::vec3 nb, glm::vec3 nc)
{
    if (glm::dot(glm::cross(b - a, c - a), na + nb + nc) < 0) {
        std::swap(b, c);
        std::swap(nb, nc);
    }
    glm::vec3 p[3] = { a, b, c }, n[3] = { na, nb, nc };
    for (int k = 0; k < 3; k++) {
        m.positions.push_back(p[k]);
        m.normals.push_back(n[k]);
    }
}

// a, b, c, d go around the quad (either direction)
static void quad(TriMesh& m, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
                 glm::vec3 na, glm::vec3 nb, glm::vec3 nc, glm::vec3 nd)
{
    tri(m, a, b, c, na, nb, nc);
    tri(m, a, c, d, na, nc, nd);
}

// upright wall from floor point p0 to p1, facing n
static void wallQuad(TriMesh& m, glm::vec3 p0, glm::vec3 p1, float height, glm::vec3 n0, glm::vec3 n1)
{
    quad(m, p0, p1, p1 + UP * height, p0 + UP * height, n0, n1, n1, n0);
}

// the same flat shape on the floor (facing up) and on the ceiling (facing down)
void Room::flatTri(glm::vec3 a, glm::vec3 b, glm::vec3 c, float height)
{
    tri(floor, a, b, c, UP, UP, UP);
    glm::vec3 h = UP * height;
    tri(ceiling, a + h, b + h, c + h, -UP, -UP, -UP);
}

Room::Room(float radius, float height, const std::vector<float>& bayAngles, GLuint program)
    : hallRadius(radius)
{
    const float R = radius, H = height;
    const float r = BAY_RADIUS;
    const float delta = std::asin(r / R);  // half the angle of hall wall each bay mouth removes

    // ---- hall: a disk of floor and ceiling
    for (int i = 0; i < SEGMENTS; i++) {
        glm::vec3 d0 = dir(2 * PI * i / SEGMENTS), d1 = dir(2 * PI * (i + 1) / SEGMENTS);
        flatTri(glm::vec3(0), d0 * R, d1 * R, H);
    }

    // ---- bays, kept in the caller's order (bays()[i] is bayAngles[i]).
    // Seen from above a bay is a U: two straight side walls going out from the hall, closed by a half circle.
    std::vector<float> angles;
    for (float deg : bayAngles) angles.push_back(glm::radians(deg));

    for (float a : angles) {
        glm::vec3 u = dir(a);                     // outward, through the middle of the bay
        glm::vec3 w = dir(a + PI / 2);            // across the bay
        glm::vec3 m = u * (R * std::cos(delta));  // middle of the mouth; m +- w*r lie exactly on the hall circle
        glm::vec3 e = m + u * BAY_DEPTH;          // center of the round end
        bayList.push_back({ e, -u });

        // straight part: side walls facing each other, floor and ceiling rectangles
        wallQuad(bayWall, m + w * r, e + w * r, H, -w, -w);
        wallQuad(bayWall, m - w * r, e - w * r, H, w, w);
        flatTri(m - w * r, m + w * r, e + w * r, H);
        flatTri(m - w * r, e + w * r, e - w * r, H);

        // round end: half circle around e from one side wall to the other, normals toward e
        const int n = 32;
        for (int i = 0; i < n; i++) {
            float t0 = PI * i / n, t1 = PI * (i + 1) / n;
            glm::vec3 e0 = w * std::cos(t0) + u * std::sin(t0), e1 = w * std::cos(t1) + u * std::sin(t1);
            wallQuad(bayWall, e + e0 * r, e + e1 * r, H, -e0, -e1);
            flatTri(e, e + e0 * r, e + e1 * r, H);
        }
    }

    // ---- hall wall: the circle minus the bay mouths, one section from each bay to the next (by angle)
    std::sort(angles.begin(), angles.end());
    std::vector<std::pair<float, float>> sections;
    if (angles.empty())
        sections.push_back({ 0.0f, 2 * PI });
    for (size_t i = 0; i < angles.size(); i++) {
        float next = (i + 1 < angles.size()) ? angles[i + 1] : angles[0] + 2 * PI;
        sections.push_back({ angles[i] + delta, next - delta });
    }
    for (auto& s : sections) {
        int n = std::max(1, (int)std::ceil((s.second - s.first) / (2 * PI) * SEGMENTS));
        for (int i = 0; i < n; i++) {
            glm::vec3 d0 = dir(s.first + (s.second - s.first) * i / n);
            glm::vec3 d1 = dir(s.first + (s.second - s.first) * (i + 1) / n);
            wallQuad(wall, d0 * R, d1 * R, H, -d0, -d1);
        }
    }

    floor.upload(program);
    ceiling.upload(program);
    wall.upload(program);
    bayWall.upload(program);
}

// Works on the floor plan (x/z): the walker is a circle, the walls are the edge of
// "hall disk + every bay's U". Each shape is shrunk by bodyRadius, so the center only has to stay inside.
glm::vec3 Room::collide(glm::vec3 pos, float bodyRadius) const
{
    glm::vec2 p(pos.x, pos.z);
    float hall = hallRadius - bodyRadius, bay = BAY_RADIUS - bodyRadius;
    if (glm::length(p) <= hall) return pos;

    // outside the hall disk: fine if inside some bay, else go to the nearest point of any shape
    glm::vec2 best = glm::normalize(p) * hall;
    for (const Bay& b : bayList) {
        // the shrunk U = points within `bay` of the segment from the mouth middle to the round end's center,
        // on the bay side of the mouth
        glm::vec2 out(-b.inward.x, -b.inward.z), across(-out.y, out.x);
        glm::vec2 end(b.center.x, b.center.z), mouth = end - out * BAY_DEPTH;
        glm::vec2 q = p - mouth;
        float along = glm::dot(q, out);

        glm::vec2 onSegment = mouth + out * glm::clamp(along, 0.0f, BAY_DEPTH);
        glm::vec2 d = (along < 0) ? across * glm::dot(q, across) : p - onSegment;  // behind the mouth: slide onto it
        if (along >= 0 && glm::length(d) <= bay) return pos;  // inside this bay
        if (glm::length(d) > bay) d = glm::normalize(d) * bay;

        glm::vec2 candidate = onSegment + d;
        if (glm::length(candidate - p) < glm::length(best - p)) best = candidate;
    }
    return glm::vec3(best.x, pos.y, best.y);
}

void Room::draw(GLuint program) const
{
    GLint color = glGetUniformLocation(program, "objectColor");
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

    const std::pair<const TriMesh*, glm::vec3> parts[] = {
        { &floor, floorColor }, { &ceiling, ceilingColor }, { &wall, wallColor }, { &bayWall, bayColor },
    };
    for (auto& p : parts) {
        glUniform3fv(color, 1, glm::value_ptr(p.second));
        p.first->draw();
    }
}

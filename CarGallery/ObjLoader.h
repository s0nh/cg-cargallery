#pragma once

#include "TriMesh.h"
#include <memory>
#include <string>
#include <vector>

// Faces of one OBJ material ("usemtl"), like tPart in the practice CObjLoader.
// Only the material NAME is kept: it says what the part is; colors are set in Car.
struct ObjPart
{
    std::string material;           // e.g. "car_main_paint", "Glass_Clear", "Thread"
    std::unique_ptr<TriMesh> mesh;  // unique_ptr: parts are moved around in a vector
};

// Minimal OBJ reader: v, vn, f, usemtl. Portable rewrite of the practice CObjLoader,
// which uses Windows-only _splitpath. .mtl files are not read (see tools/gltf2obj.py).
class ObjLoader
{
public:
    std::vector<ObjPart> parts;
    glm::vec3 bboxMin = glm::vec3(0.0f);
    glm::vec3 bboxMax = glm::vec3(0.0f);

    bool load(const std::string& objPath);

private:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;

    int partFor(const std::string& material);  // index of the part, created if new
};

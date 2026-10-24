#include "ObjLoader.h"
#include <fstream>
#include <iostream>
#include <sstream>

bool ObjLoader::load(const std::string& objPath)
{
    std::ifstream file(objPath);
    if (!file) {
        std::cerr << "Failed to open " << objPath << std::endl;
        return false;
    }
    // an index, not a pointer: adding a part can move the vector's elements
    int current = partFor("default");
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream in(line);
        std::string tag;
        in >> tag;

        if (tag == "v") {
            glm::vec3 p;
            in >> p.x >> p.y >> p.z;
            bboxMin = positions.empty() ? p : glm::min(bboxMin, p);
            bboxMax = positions.empty() ? p : glm::max(bboxMax, p);
            positions.push_back(p);
        }
        else if (tag == "vn") {
            glm::vec3 n;
            in >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (tag == "usemtl") {
            std::string name;
            in >> name;
            current = partFor(name);
        }
        else if (tag == "f") {
            // corners look like "v", "v/vt", "v//vn" or "v/vt/vn"; OBJ counts from 1
            std::vector<int> v, vn;
            std::string corner;
            while (in >> corner) {
                v.push_back(std::stoi(corner) - 1);
                size_t first = corner.find('/'), last = corner.rfind('/');
                bool hasNormal = first != last && last + 1 < corner.size();  // two slashes and something after
                vn.push_back(hasNormal ? std::stoi(corner.substr(last + 1)) - 1 : -1);
            }
            // polygon -> triangle fan (0,1,2), (0,2,3), ...
            for (size_t i = 1; i + 1 < v.size(); i++) {
                int tri[3] = { 0, (int)i, (int)i + 1 };
                glm::vec3 a = positions[v[tri[0]]], b = positions[v[tri[1]]], c = positions[v[tri[2]]];
                glm::vec3 faceN = glm::normalize(glm::cross(b - a, c - a));  // used when the file has no vn
                TriMesh& mesh = *parts[current].mesh;
                for (int k : tri) {
                    mesh.positions.push_back(positions[v[k]]);
                    mesh.normals.push_back(vn[k] >= 0 ? normals[vn[k]] : faceN);
                }
            }
        }
    }
    return true;
}

int ObjLoader::partFor(const std::string& material)
{
    for (size_t i = 0; i < parts.size(); i++)
        if (parts[i].material == material) return (int)i;
    parts.push_back(ObjPart());
    parts.back().material = material;
    parts.back().mesh.reset(new TriMesh());
    return (int)parts.size() - 1;
}

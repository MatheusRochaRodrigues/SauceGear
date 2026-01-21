#pragma once
#include <array>
#include <memory>
#include <glm/glm.hpp>

// -----------------------------
// Nó de uma Octree genérica
// -----------------------------
template<typename T>
class OctreeNode {
public:
    glm::vec3 center;
    float size;
    T data;

    std::array<std::unique_ptr<OctreeNode<T>>, 8> children;

    OctreeNode(const glm::vec3& c, float s) : center(c), size(s), data{} { }

    bool isLeaf() const { return children[0] == nullptr; }

    void subdivide() {
        if (!isLeaf()) return;
        float childSize = size * 0.5f;
        int idx = 0;
        for (int x = -1; x <= 1; x += 2)
            for (int y = -1; y <= 1; y += 2)
                for (int z = -1; z <= 1; z += 2) {
                    glm::vec3 childCenter = center + glm::vec3(x, y, z) * (childSize * 0.5f);
                    children[idx++] = std::make_unique<OctreeNode<T>>(childCenter, childSize);
                }
    }
};

#pragma once
#include "OctreeNode.h"

template<typename T>
class Octree {
public:
    std::unique_ptr<OctreeNode<T>> root;
    int maxDepth;

    Octree(const glm::vec3& center, float size, int depth = 1) {
        root = std::make_unique<OctreeNode<T>>(center, size);
        maxDepth = depth;
        if (depth > 0) subdivide(root.get(), depth);
    }

    OctreeNode<T>* findLeaf(const glm::vec3& point) {
        return findLeafRecursive(root.get(), point, maxDepth);
    }

    // Itera todas as folhas para Surface Nets
    template<typename Func>
    void forEachLeaf(Func&& f) {
        traverseLeaves(root.get(), f);
    }

private:
    void subdivide(OctreeNode<T>* node, int depth) {
        if (depth <= 0) return;
        node->subdivide();
        for (auto& child : node->children)
            subdivide(child.get(), depth - 1);
    }

    OctreeNode<T>* findLeafRecursive(OctreeNode<T>* node, const glm::vec3& point, int depth) {
        if (!node || depth == 0 || node->isLeaf()) return node;

        int idx = 0;
        idx |= (point.x > node->center.x) ? 1 : 0;
        idx |= (point.y > node->center.y) ? 2 : 0;
        idx |= (point.z > node->center.z) ? 4 : 0;

        return findLeafRecursive(node->children[idx].get(), point, depth - 1);
    }

    template<typename Func>
    void traverseLeaves(OctreeNode<T>* node, Func&& f) {
        if (!node) return;
        if (node->isLeaf()) {
            f(node);
            return;
        }
        for (auto& c : node->children) {
            traverseLeaves(c.get(), f);
        }
    }
};

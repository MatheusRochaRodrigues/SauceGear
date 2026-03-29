#include <iostream>
#include <glm/glm.hpp>

using namespace glm;

/*
struct EdgeKey
{
    ivec3 pos;
    int axis;

    bool operator==(const EdgeKey& o) const
    {
        return pos == o.pos && axis == o.axis;
    }
};

struct EdgeHasher
{
    size_t operator()(const EdgeKey& k) const
    {
        size_t h = std::hash<int>()(k.pos.x);
        h ^= std::hash<int>()(k.pos.y) << 1;
        h ^= std::hash<int>()(k.pos.z) << 2;
        h ^= std::hash<int>()(k.axis) << 3;
        return h;
    }
};

struct EdgeData
{
    vec3 position;
    vec3 normal;
};

std::unordered_map<EdgeKey, EdgeData, EdgeHasher> edgeCache;
std::shared_mutex edgeMutex;





EdgeData SampleEdge(const ivec3& pos, int axis)
{
    EdgeKey key{ pos, axis };

    {
        std::shared_lock lock(edgeMutex);

        auto it = edgeCache.find(key);
        if (it != edgeCache.end())
            return it->second;
    }

    EdgeData data;

    // calcular interseção real aqui
    data.position = ComputeIntersection(pos, axis);
    data.normal = ComputeNormal(pos);

    {
        std::unique_lock lock(edgeMutex);
        edgeCache[key] = data;
    }

    return data;
}
*/
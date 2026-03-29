#pragma once 
#include <glm/glm.hpp>

using namespace glm;

constexpr int BRICK_SIZE = 15;   //15 + 1 == 16   //16

//struct SDFBrick
//{
//    float density[BRICK_SIZE + 1][BRICK_SIZE + 1][BRICK_SIZE + 1];
//};

struct SDFBrick
{
    std::vector<float> density;

    SDFBrick() {
        density.resize((BRICK_SIZE + 1) * (BRICK_SIZE + 1) * (BRICK_SIZE + 1));
    }
};


struct BrickKey
{
    ivec3 coord;

    bool operator==(const BrickKey& o) const
    {
        return coord == o.coord;
    }
};

struct BrickHasher
{
    size_t operator()(const BrickKey& k) const
    {
        size_t h = std::hash<int>()(k.coord.x);
        h ^= std::hash<int>()(k.coord.y) << 1;
        h ^= std::hash<int>()(k.coord.z) << 2;
        return h;
    }
};













/*
inline uint32_t NextPowerOfTwo(uint32_t v)
{
    if (v == 0) return 1;

    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;

    return v;
}
*/
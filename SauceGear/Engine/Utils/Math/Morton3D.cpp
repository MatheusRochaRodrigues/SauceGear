#include "Morton3D.h"

uint32_t Part1By2(uint32_t x)
{
    x &= 0x000003ff;
    x = (x ^ (x << 16)) & 0xff0000ff;
    x = (x ^ (x << 8))  & 0x0300f00f;
    x = (x ^ (x << 4))  & 0x030c30c3;
    x = (x ^ (x << 2))  & 0x09249249;
    return x;
}

uint32_t Morton3D(uint32_t x, uint32_t y, uint32_t z)
{
    return (Part1By2(z) << 2) | (Part1By2(y) << 1) | Part1By2(x);
}



/*alternative

inline uint32_t Morton3D(uint32_t x, uint32_t y, uint32_t z)
{
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x << 8))  & 0x0300F00F;
    x = (x | (x << 4))  & 0x030C30C3;
    x = (x | (x << 2))  & 0x09249249;

    y = (y | (y << 16)) & 0x030000FF;
    y = (y | (y << 8))  & 0x0300F00F;
    y = (y | (y << 4))  & 0x030C30C3;
    y = (y | (y << 2))  & 0x09249249;

    z = (z | (z << 16)) & 0x030000FF;
    z = (z | (z << 8))  & 0x0300F00F;
    z = (z | (z << 4))  & 0x030C30C3;
    z = (z | (z << 2))  & 0x09249249;

    return x | (y << 1) | (z << 2);
}

*/




/*

#pragma once
#include <cstdint>

inline uint32_t expandBits(uint32_t v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

inline uint32_t morton3D(uint32_t x, uint32_t y, uint32_t z)
{
    return (expandBits(x) << 2) |
           (expandBits(y) << 1) |
            expandBits(z);
}

*/
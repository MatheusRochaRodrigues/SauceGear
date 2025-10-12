#pragma once
#include "../../Geometry/World/SurfaceNets/SurfaceNets.h"
#include "../Graphics/Mesh.h"

struct SurfaceNetsComponent { 
    Chunk* chunk; 
    bool dirty = true; // força regeneração

    SurfaceNetsComponent(Chunk* chunk) : chunk(chunk) {}
    SurfaceNetsComponent() {}
};

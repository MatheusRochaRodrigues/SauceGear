#pragma once
#include "../../Geometry/World/SurfaceNets/SurfaceNets.h"
#include "../Graphics/Mesh.h"

struct SurfaceNetsComponent {
    SurfaceNetsParams* params;
    SurfaceNetsBuffer* buffer;
    //VoxelGrid grid; // atualizada pelo usuário/sistema
    //Mesh* mesh = nullptr;
    bool dirty = true; // força regeneração

    //SurfaceNetsComponent(int sx = 0, int sy = 0, int sz = 0) : grid(sx, sy, sz) {}
    /*SurfaceNetsComponent(SurfaceNetsParams& params, SurfaceNetsBuffer& buffer)  {
        this->params = &params;
        this->buffer = &buffer;
    };*/
};

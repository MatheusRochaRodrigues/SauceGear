#pragma once  
#include <glm/glm.hpp>
#include "QEF/qef.h"

using namespace glm;

struct OctreeDrawInfo
{
    OctreeDrawInfo() : index(-1), corners(0) {}

    int             index;          // índice no vertex buffer 
    float           cornersDens[8]; // sinais dos 8 cantos
    int             corners;        // sinais dos 8 cantos
    vec3            position;       // vértice QEF
    vec3            averageNormal;  // normal média
    svd::QefData    qef;            // dados acumulados do QEF
};

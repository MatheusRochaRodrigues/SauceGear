#pragma once  
#include <glm/glm.hpp>  
#include "../ECS/Reflection/Macros.h" 
 
struct Chunk;
struct OctreeNode;

struct SurfaceNetsComponent { 
    Chunk* chunk;
    OctreeNode* node;
    bool dirty = true; // força regeneração

    bool showBoxOctree = false;
    glm::vec3 colorBox = glm::vec3(1.0f, 0, 1.0f); 

    REFLECT_CLASS(SurfaceNetsComponent) {
        REFLECT_HEADER("SurfaceNetsComponent");
        REFLECT_FIELD(showBoxOctree);
        REFLECT_FIELD(colorBox);
         
    }

    SurfaceNetsComponent(Chunk* chunk) : chunk(chunk) {}
    SurfaceNetsComponent(Chunk* chunk, OctreeNode* node) : chunk(chunk), node(node) {}
    SurfaceNetsComponent() {}
};

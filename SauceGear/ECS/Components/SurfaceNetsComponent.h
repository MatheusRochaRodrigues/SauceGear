#pragma once  
#include <vector>
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
     
    SurfaceNetsComponent(Chunk* chunk) : chunk(chunk) {}
    SurfaceNetsComponent(Chunk* chunk, OctreeNode* node) : chunk(chunk), node(node) {}
    SurfaceNetsComponent(Chunk* chunk, OctreeNode* node, glm::vec3 center) : chunk(chunk), node(node), center(center) {}
    SurfaceNetsComponent() {}

    //Debug
    glm::vec3 center;  
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> pointsDeep;
    int lod; 

    REFLECT_CLASS(SurfaceNetsComponent) {
        REFLECT_HEADER("SurfaceNetsComponent");
        REFLECT_FIELD(showBoxOctree);
        REFLECT_FIELD(colorBox);
        REFLECT_HEADER("CenterNode");
        REFLECT_FIELD(center);
        REFLECT_VECTOR(points, glm::vec3);
        REFLECT_VECTOR(pointsDeep, glm::vec3);

        REFLECT_FIELD(lod);

    }
};

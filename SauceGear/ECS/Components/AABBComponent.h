#pragma once
#include <glm/glm.hpp>  

struct AABBComponent {
    glm::vec3 localMin;
    glm::vec3 localMax;

    glm::vec3 worldMin;
    glm::vec3 worldMax;

    bool dirty = true; // caso Mesh,Collider, Voxel chunk foi rebuildado, Bounds local mudou e etc coisas assim foram modificadas
    bool dirtyLocal = true;

    void setLocal(glm::vec3 min, glm::vec3 max) {
        localMin = min; 
        localMax = max;
    }
    void setWorld(glm::vec3 min, glm::vec3 max) { 
        worldMin = min;
        worldMax = max;
    }

};


//struct AABBComponent {
//    AABB local;   // calculado UMA VEZ a partir do mesh
//    AABB world;   // atualizado quando transform muda
//};



//struct AABBComponent {
//    glm::vec3 min = glm::vec3(FLT_MAX);
//    glm::vec3 max = glm::vec3(-FLT_MAX);
//};



//struct BoundingSphereComponent {
//    glm::vec3 center = glm::vec3(0.0f);
//    float radius = 0.0f;
//};


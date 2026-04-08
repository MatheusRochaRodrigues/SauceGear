#pragma once 
#include <iostream>
#include <vector>
#include <queue>
#include <atomic>
#include <memory>
#include <glm/glm.hpp>

#include "clipmap_system.cpp"
#include "../Core/Camera.h"
#include "../Scene/SceneECS.h"
#include "ChunkStreamingBridge.h"

using namespace glm;  

class World {
public: 
    // SYSTEMS 
    ClipmapSystem clipmap;  
      
    void Initialize(Scene& scene)
    {
        clipmap.Initialize(); 

        auto bridge = scene.EmplaceResource<ChunkStreamingBridge>(); 
        clipmap.bridge.ECSBridge = bridge.get();         //clipmap.readyChunks = &bridge->readyChunks;
    }
     

    void Update(Camera* cam)
    {
        const vec3& cameraPos = cam->GetPosition();
        const vec3& cameraForward = cam->Front;
        
        clipmap.Update(cameraPos, cameraForward);
        //UpdateStreaming(cameraPos, cameraForward); 
        //ProcessStitching(); // hook futuro
    }
     
};
 

#pragma once
#include "../../Instancing/ModelInstance.h"
#include "../../Instancing/MeshInstance.h"
 
struct ModelComponent {
    //std::shared_ptr<ModelInstance> instance;
    std::shared_ptr<MeshInstance> instance;
};
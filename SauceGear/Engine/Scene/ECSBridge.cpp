#include "ECSBridge.h"
#include "SceneECS.h"

void ECSBridge::AddComponent(void* scene, Entity e, std::type_index type) {
    auto* ecs = static_cast<SceneECS*>(scene);
    assert(ecs);
    ecs->AddComponentByType_Internal(e, type);
}

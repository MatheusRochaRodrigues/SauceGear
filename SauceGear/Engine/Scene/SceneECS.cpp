#include "SceneECS.h" 

#include "../ECS/Systems/RenderSystem.h" 
#include "../ECS/Systems/MoveSystem.h"
#include "../ECS/Systems/CameraSystem.h"
#include "../ECS/Systems/PostProcessSystem.h" 
#include "../ECS/Systems/GlobalUniformSystem.h" 
#include "../ECS/Systems/PickingSystem.h" 
#include "../ECS/Systems/DayNightSystem.h"  
#include "../ECS/Systems/ComputeSyncSystem.h"  
#include "../ECS/Systems/TransformSystem.h"  
#include "../ECS/Systems/EndLoopSystem.h" 
#include "../ECS/Systems/AABBSystem.h"   
#include "../ECS/Systems/DebugRenderer.h"   
#include "../ECS/Systems/TextRender/TextRenderSystem.h"  

#include "../ECS/Components/ComponentsHelper.h" 

void SceneECS::initECS() {
    ///----------Components
    //auto* physics = scene.RegisterSystem<PhysicsSystem>();
    componentManager->Register<CameraComponent>();
    componentManager->Register<TransformComponent>(); 
    componentManager->Register<MeshRenderer>();
    componentManager->Register<LightComponent>();
    componentManager->Register<HierarchyComponent>();
    componentManager->Register<NameComponent>();
    //componentManager->Register<PostProcessComponent>(); 
    componentManager->Register<AABBComponent>(); 
    componentManager->Register<SurfaceNetsComponent>();
    componentManager->Register<DebugMeshComponent>();
    componentManager->Register<TextComponent>();


    ///----------Systems
    RegisterSystem <TransformSystem>();
    RegisterSystem <AABBSystem>();
    RegisterSystem <CameraSystem>();
    RegisterSystem <ComputeSyncSystem>();
    RegisterSystem <DayNightSystem>(); 
    RegisterSystem <GlobalUniformSystem>(); 
    RegisterSystem <RenderSystem>(); 
    RegisterSystem <DebugRenderer>();
    RegisterSystem <PostProcessSystem>();
    RegisterSystem <TextRenderSystem>();
    RegisterSystem <PickingSystem>();

    //RegisterSystem <OctreeWorldSystem>();  

    //auto* moveSystem = scene.RegisterSystem<MoveSystem>();
    //auto* inputSystem = RegisterSystem <InputSystem>(); 

    RegisterSystem <EndLoopSystem>();
}

Entity SceneECS::CreateEntity() {
    return entityManager.CreateEntity();
} 

void SceneECS::DestroyEntity(Entity entity) {
    return entityManager.DestroyEntity(entity);
}


void SceneECS::AddToParent(Entity father, Entity child) {
    if (!HasComponent<HierarchyComponent>(father))
        AddComponent<HierarchyComponent>(father);

    if (!HasComponent<HierarchyComponent>(child))
        AddComponent<HierarchyComponent>(child);

    auto& parentH = GetComponent<HierarchyComponent>(father);
    auto& childH = GetComponent<HierarchyComponent>(child);
    childH.parent = father;
    if (parentH.firstChild == INVALID_ENTITY) {
        parentH.firstChild = child;
    }
    else {
        Entity current = parentH.firstChild;
        HierarchyComponent* currentH = &GetComponent<HierarchyComponent>(current);
        while (currentH->nextSibling != INVALID_ENTITY) {
            current = currentH->nextSibling;
            currentH = &GetComponent<HierarchyComponent>(current);
        }
        currentH->nextSibling = child;
    }
}

//Entity parent = SceneBuilder::CreateGameObject("parent 2");
//Entity child1 = SceneBuilder::CreateGameObject("Child 2");
//AddToParent(parent, child1);



//Entity SceneECS::FindEntityByName(const std::string& name) {
//    auto entities = GetEntitiesWith<NameComponent>();
//    for (Entity e : entities) {
//        auto& comp = GetComponent<NameComponent>(e);
//        if (comp.name == name) {
//            return e;
//        }
//    }
//    return INVALID_ENTITY;
//}


//template<typename... Components>
//std::vector<Entity> SceneECS::GetEntitiesWith() {
//    /*std::vector<Entity> matchingEntities;
//    for (Entity entity : entityManager.GetAllEntities()) {
//        if ((HasComponent<Components>(entity) && ...)) {
//            matchingEntities.push_back(entity);
//        }
//    }
//    return matchingEntities;*/
//
//    return componentManager->GetEntitiesWith<Components...>();
//}


//unsigned int SceneECS::CreateEntity() {
//    // sua lógica aqui, exemplo:
//    static unsigned int nextId = 1;
//    return nextId++;
//}
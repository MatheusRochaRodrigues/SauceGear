#include "SceneECS.h"  

// Systems
#include "../ECS/Systems/RenderSystem.h" 
#include "../ECS/Systems/MoveSystem.h"
#include "../ECS/Systems/CameraSystem.h" 
#include "../ECS/Systems/GlobalUniformSystem.h" 
#include "../ECS/Systems/PickingSystem.h" 
#include "../ECS/Systems/DayNightSystem.h"  
#include "../ECS/Systems/ComputeSyncSystem.h"  
#include "../ECS/Systems/TransformSystem.h"  
#include "../ECS/Systems/EndLoopSystem.h" 
#include "../ECS/Systems/AABBSystem.h"   
#include "../ECS/Systems/DebugRenderer.h"   
#include "../ECS/Systems/TextRender/TextRenderSystem.h"  
// Components
#include "../ECS/Components/TransformComponent.h"
#include "../ECS/Components/MeshRenderer.h"  
#include "../ECS/Components/Velocity.h" 
#include "../ECS/Components/LightComponent.h" 
#include "../ECS/Components/CameraComponent.h"  
#include "../ECS/Components/GlobalUniformComponent.h"  
//#include "../ECS/Components/NativeScriptComponent.h"  
#include "../ECS/Components/HierarchyComponent.h"  
#include "../ECS/Components/AABBComponent.h"  
#include "../ECS/Components/ComputeSyncComponent.h"   
#include "../ECS/Components/SurfaceNetsComponent.h"   
#include "../ECS/Components/DebugMeshComponent.h"   
#include "../ECS/Components/TextComponent.h"  


void SceneECS::initECS() {
    ///----------Components
    //auto* physics = scene.RegisterSystem<PhysicsSystem>();
    componentManager->Register<CameraComponent>();
    componentManager->Register<TransformComponent>(); 
    componentManager->Register<MeshRenderer>();
    componentManager->Register<LightComponent>();
    componentManager->Register<HierarchyComponent>();
    componentManager->Register<NameComponent>(); 
    componentManager->Register<AABBComponent>(); 
    componentManager->Register<SurfaceNetsComponent>();
    componentManager->Register<DebugMeshComponent>();
    componentManager->Register<TextComponent>();


    ///----------Systems
    RegisterSystem <TransformSystem>();
    RegisterSystem <AABBSystem>();
    RegisterSystem <CameraSystem>();
    RegisterSystem <ComputeSyncSystem>();
    RegisterSystem <PickingSystem>();
    RegisterSystem <DayNightSystem>(); 
    RegisterSystem <GlobalUniformSystem>(); 
    RegisterSystem <RenderSystem>(); 
    RegisterSystem <DebugRenderer>();
    RegisterSystem <TextRenderSystem>();

    //RegisterSystem <OctreeWorldSystem>();  

    //auto* moveSystem = scene.RegisterSystem<MoveSystem>();
    //auto* inputSystem = RegisterSystem <InputSystem>(); 

    RegisterSystem <EndLoopSystem>();
}

Entity SceneECS::CreateEntity() {
    return entityManager.CreateEntity();
} 


void SceneECS::DestroyEntity(Entity entity) {
    if (!entityManager.Exists(entity)) return;

    LightPass::OnEntityDestroyed(entity);  //Destroy Light


    // 1️ Remove da hierarquia (pai / filhos)
    if (HasComponent<HierarchyComponent>(entity)) {
        auto& h = GetComponent<HierarchyComponent>(entity);

        // Remove referencia do pai
        if (h.parent != INVALID_ENTITY && HasComponent<HierarchyComponent>(h.parent)) {
            auto& parent = GetComponent<HierarchyComponent>(h.parent);

            Entity child = parent.firstChild;
            Entity prev = INVALID_ENTITY;

            while (child != INVALID_ENTITY) {
                if (child == entity) {
                    if (prev == INVALID_ENTITY)
                        parent.firstChild = h.nextSibling;
                    else
                        GetComponent<HierarchyComponent>(prev).nextSibling = h.nextSibling;
                    break;
                }
                prev = child;
                child = GetComponent<HierarchyComponent>(child).nextSibling;
            }
        }

        // Remove filhos recursivamente
        Entity child = h.firstChild;
        while (child != INVALID_ENTITY) {
            Entity next = GetComponent<HierarchyComponent>(child).nextSibling;
            DestroyEntity(child);
            child = next;
        }
    }

    // 2️ Remove TODOS os componentes
    const auto& storages = componentManager->GetAllStorages();
    for (auto& [type, storage] : storages) {
        if (storage->Has(entity))
            storage->Remove(entity);
    }

    // 3️ Remove entidade
    entityManager.DestroyEntity(entity);

    // 4️ Limpa seleção
    if (selectedEntity == entity)
        selectedEntity = INVALID_ENTITY;
}


void SceneECS::AddComponentByType(Entity e, std::type_index type) {
    auto it = componentManager->GetAllStorages().find(type);
    if (it == componentManager->GetAllStorages().end())
        return;

    if (auto* ti = ReflectionRegistry::Get().Get(type)) {
        if (ti->Add) {
            ti->Add(this, e);
        }
    }
}

void SceneECS::AddComponentByType_Internal(Entity e, std::type_index type) {
    auto& storages = componentManager->GetAllStorages();
    auto it = storages.find(type);
    if (it == storages.end())
        return;

    // NÃO chama TypeInfo::Add
    it->second->EmplaceDefault(e);
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
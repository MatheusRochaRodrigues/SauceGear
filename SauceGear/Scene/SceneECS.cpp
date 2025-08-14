#include "SceneECS.h"
#include "../Scene/Systems/SystemHelper.h"


void SceneECS::initECS() {
    ///----------Components
    //auto* physics = scene.RegisterSystem<PhysicsSystem>();
    componentManager->Register<CameraComponent>();
    componentManager->Register<Transform>();
    componentManager->Register<Material>();
   
    componentManager->Register<MeshFilter>();

    componentManager->Register<MeshRenderer>();
    componentManager->Register<LightComponent>();
    componentManager->Register<HierarchyComponent>();
    componentManager->Register<NameComponent>();
    //componentManager->Register<PostProcessComponent>();

    ///----------Systems
    auto* cameraSystem = RegisterSystem <CameraSystem>();
    auto* lightSystem  = RegisterSystem <LightSystem> ();
    auto* globalUniformSystem = RegisterSystem <GlobalUniformSystem>();
    auto* renderSystem = RegisterSystem <RenderSystem>();
    auto* postProcessSystem = RegisterSystem <PostProcessSystem>();

    //auto* moveSystem = scene.RegisterSystem<MoveSystem>();
    //auto* inputSystem = RegisterSystem <InputSystem>();

}

Entity SceneECS::CreateEntity() {
    return entityManager.CreateEntity();
}

Entity SceneECS::NewGameObj(string name) {
    Entity entity = entityManager.CreateEntity(); 

    //Special Components
    AddComponent<NameComponent>(entity).name = name;                      //AddComponent<NameComponent>(entity, name);
    
    AddComponent<Transform>(entity);

    return entity;
}


void SceneECS::DestroyEntity(Entity entity) {
    return entityManager.DestroyEntity(entity);
}


void SceneECS::AddToParent(Entity parent, Entity child) {
    if (!HasComponent<HierarchyComponent>(parent))
        AddComponent<HierarchyComponent>(parent);

    if (!HasComponent<HierarchyComponent>(child))
        AddComponent<HierarchyComponent>(child);

    auto& parentH = GetComponent<HierarchyComponent>(parent);
    auto& childH = GetComponent<HierarchyComponent>(child);
    childH.parent = parent;
    if (parentH.firstChild == INVALID_ENTITY) {
        parentH.firstChild = child;
    }
    else {
        Entity current = parentH.firstChild;
        auto& currentH = GetComponent<HierarchyComponent>(current);
        while (currentH.nextSibling != INVALID_ENTITY) {
            current = currentH.nextSibling;
            currentH = GetComponent<HierarchyComponent>(current);
        }
        currentH.nextSibling = child;
    }
}







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
#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <cassert>
 
#include "../Scene/ComponentManager.h"
#include "../ECS/System.h"  
#include "../ECS/Reflection/Macros.h" 

class SceneECS {
public:
    void initECS();
    //SceneECS();

    // Entidades 
    Entity CreateEntity();
    void DestroyEntity(Entity entity); 

    // Componentes
    template<typename T, typename... Args>
    T& AddComponent(Entity entity, Args&&... args) {
        if (!componentManager) throw std::runtime_error("componentManager is nullptr");

        try { 
            //return componentManager->AddComponent<T>(entity, std::forward<Args>(args)...); 
            T& comp = componentManager->AddComponent<T>(entity, std::forward<Args>(args)...); 
            std::cout << "+ Registrado " << typeid(T).name() << "\n";
            return comp;
        } catch (const std::exception& e) {
            std::cerr << "[ERRO] AddComponent em Scene falhou para tipo " << typeid(T).name() << ": " << e.what() << "\n";
            throw; // repropaga para pegar mais acima se quiser
        }
    }

    template<typename T>
    void RemoveComponent(Entity entity) {
        componentManager->RemoveComponent<T>(entity);
    }

    void RemoveComponent(Entity entity, const TypeInfo& type) {
        const auto& storages = componentManager->GetAllStorages();
        auto it = storages.find(type.typeIndex);
        if (it != storages.end()) {
            it->second->Remove(entity);
            return;
        }
        
        std::cout << "[Error] nao foi possivel remover o Componente (não existe)" << std::endl;
        //assert(false && "TransformComponent cannot be removed");
    }


    template<typename T>
    bool HasComponent(Entity entity) const {
        return componentManager->HasComponent<T>(entity);
    }

    template<typename T>
    T& GetComponent(Entity entity) {
        assert(HasComponent<T>(entity));
        return componentManager->GetComponent<T>(entity);           // scene.GetComponent<Transform>(e)     // assert no debug
    }
    //  auto& t = scene.GetComponent<Transform>(e);                 

    template<typename T>
    T* TryGetComponent(Entity entity) {                           
        return componentManager->TryGetComponent<T>(entity);        //  if (auto* t = scene.TryGetComponent<Transform>(e))     // seguro        
    }
     

    // Utilidade: obter entidades com um conjunto de componentes
    template<typename... Components>
    std::vector<Entity> GetEntitiesWith() {
        return componentManager->GetEntitiesWith<Components...>();
    };

    // Sistemas
    template<typename T, typename... Args>
    T* RegisterSystem(Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = system.get();
        systems.emplace_back(std::move(system));

        std::cerr << "[INFO - Sys] "
            << typeid(T).name()
            << "\n";


        return ptr;
    }

    void Update(float deltaTime) {  //dt == deltaTime
        for (auto& system : systems) {
            system->Update(deltaTime);
        }
    }

    void SelectEntity(Entity entity) { selectedEntity = entity; }
    Entity GetSelectedEntity() const { return selectedEntity; }

    virtual void Load(){};


    // Método para pegar todas entidades ativas
    std::vector<Entity> GetAllEntities() const {
        return entityManager.GetAllEntities();
    }

    // Exemplo básico: implementa por entidade
    //void DrawComponents(Entity entity) {
    //    if (HasComponent<Transform>(entity)) {
    //        auto& transform = GetComponent<Transform>(entity);
    //        if (ImGui::TreeNode("Transform")) {
    //            ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
    //            ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f);
    //            ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
    //            ImGui::TreePop();
    //        }
    //    }

    //    // outros componentes...
    //}

    void AddToParent(Entity, Entity); 

    bool HasComponentType(Entity e, std::type_index type) const {
        return componentManager->HasComponentType(e, type);
    }
    std::vector<std::type_index> GetComponentTypes(Entity e) const {
        return componentManager->GetComponentTypes(e);
    } 

    // Retorna um vetor de tipos de componentes da entidade
    // Retorna os tipos refletidos que a entidade realmente possui
    std::vector<TypeInfo*> GetComponentTypes(Entity entity) {
        std::vector<TypeInfo*> result;

        const auto& storages = componentManager->GetAllStorages();
        for (const auto& [typeIdx, storagePtr] : storages) {
            if (storagePtr->Has(entity)) {
                if (auto* ti = ReflectionRegistry::Get().Get(typeIdx)) {
                    result.push_back(ti);
                }
            }
        }
        return result;
    } 
     
    // Retorna ponteiro cru para o componente
    void* GetComponentRaw(Entity entity, const TypeInfo& type) {
        const auto& storages = componentManager->GetAllStorages();
        auto it = storages.find(type.typeIndex);
        if (it == storages.end()) return nullptr;

        IComponentStorage* storage = it->second.get(); // pega o ponteiro cru
        return storage->GetRaw(entity);
    } 

    //Entity FindEntityByName(const std::string& name);
    Entity computeManager = INVALID_ENTITY; // global para syncs 

    //void SwitchToCamera(Camera* newCam) { GEngine->mainCamera = newCam; }

    template<typename T>
    Entity GetFirstEntityOfType() const {
        return componentManager->GetFirstEntityOfType<T>();
    }

    bool EntityExists(Entity e) const {
        return entityManager.Exists(e); // delega para o EntityManager
    }

    void AddComponentByType(Entity e, std::type_index type);

    bool CanAddComponentType(std::type_index type) const {
        return componentManager->GetAllStorages().count(type) != 0;
    }

    void AddComponentByType_Internal(Entity e, std::type_index type);

    //----------------------------------------------------------------------------
    //  RESOURCES
    //----------------------------------------------------------------------------

    /*template<typename T>
    void SetResource(std::shared_ptr<T> resource)
    {
        resources[typeid(T)] = resource;
    }*/

    /*
    template<typename T>
    void SetResource(T* resource)
    {
        resources[typeid(T)] = std::shared_ptr<T>(resource);
    }
    */

    /*
    template<typename T>
    void SetResource(T resource)
    {
        resources[typeid(T)] = std::make_shared<T>(std::move(resource));
    }
    */

    template<typename T, typename... Args>
    std::shared_ptr<T> EmplaceResource(Args&&... args)
    {
        auto resource = std::make_shared<T>(std::forward<Args>(args)...);
        resources[typeid(T)] = resource;
        return resource;

        //auto bridge = scene.EmplaceResource<ChunkStreamingBridge>();   way use example
    }

    /*template<typename T>
    T& GetResource()
    {
        auto it = resources.find(typeid(T));
        if (it == resources.end())
            throw std::runtime_error("Resource not found");

        return *std::static_pointer_cast<T>(it->second);
    }*/

    template<typename T>
    std::shared_ptr<T> GetResource()
    {
        auto it = resources.find(typeid(T));
        if (it == resources.end())
            throw std::runtime_error("Resource not found");

        return std::static_pointer_cast<T>(it->second);
    }

    template<typename T>
    std::shared_ptr<T> TryGetResource()
    {
        auto it = resources.find(typeid(T));
        if (it == resources.end())
            return nullptr;

        return std::static_pointer_cast<T>(it->second);
    }

    template<typename T>
    T* TryGetResourceRaw()      //if you want to avoid the cost of the shared_ptr, Pointer Raw is lighter.
    {
        auto it = resources.find(typeid(T));
        if (it == resources.end())
            return nullptr;

        return static_cast<T*>(it->second.get());
    }

    void DestroyGameObject(Entity e) {
        toDestroy.push_back(e);
    }

private:
    std::unique_ptr<ComponentManager> componentManager = std::make_unique<ComponentManager>(); 
    std::vector<std::unique_ptr<System>> systems;
    EntityManager entityManager;

    std::unordered_map<std::type_index, std::shared_ptr<void>> resources;

    Entity selectedEntity = INVALID_ENTITY;


    std::vector<Entity> toDestroy;
};

 
using Scene = SceneECS;





//estudar funcionamento
//template<>
//MeshRenderer& SceneECS::AddComponent<MeshRenderer>(Entity entity) {
//    MeshRenderer mr;
//    mr.material = DefaultResources::GetDefaultMaterial();
//    return componentManager->AddComponent<MeshRenderer>(entity, std::move(mr));
//}









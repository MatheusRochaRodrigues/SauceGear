#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <type_traits>
 
#include "../Scene/ComponentManager.h"
#include "../ECS/System.h" 
#include "../Resources/Primitive.h"
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
        }
    }


    template<typename T>
    bool HasComponent(Entity entity) const {
        return componentManager->HasComponent<T>(entity);
    }

    template<typename T>
    T& GetComponent(Entity entity) {
        return componentManager->GetComponent<T>(entity);
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

private:
    std::unique_ptr<ComponentManager> componentManager = std::make_unique<ComponentManager>(); 
    std::vector<std::unique_ptr<System>> systems;
    EntityManager entityManager;

    Entity selectedEntity = INVALID_ENTITY;
};








//estudar funcionamento
//template<>
//MeshRenderer& SceneECS::AddComponent<MeshRenderer>(Entity entity) {
//    MeshRenderer mr;
//    mr.material = DefaultResources::GetDefaultMaterial();
//    return componentManager->AddComponent<MeshRenderer>(entity, std::move(mr));
//}









#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <iostream>

#include "Entity.h"

class IComponentStorage {
public:
    virtual ~IComponentStorage() = default;
    virtual void Remove(Entity entity) = 0;
    virtual bool Has(Entity entity) const = 0;

    //acesso void* universal.
    virtual void* GetRaw(Entity e) = 0;
};

template<typename T>
class ComponentStorage : public IComponentStorage {
public:
    void Add(Entity entity, T component) {
        components[entity] = component;
        entities.insert(entity);
    }

    void Remove(Entity entity) override {
        components.erase(entity);
        entities.erase(entity);
    }

    bool Has(Entity entity) const override {
        return components.find(entity) != components.end();
    }

    T& Get(Entity entity) {
        return components.at(entity);
    }

    void* GetRaw(Entity entity) override {
        return &components.at(entity); // ponteiro cru
    }

    std::unordered_set<Entity>& GetEntitySet() {
        return entities;
    }

private:
    std::unordered_map<Entity, T> components;
    std::unordered_set<Entity> entities;
};



class ComponentManager {
public:
    // Registra um componente
    template<typename T>
    void Register() {
        std::type_index type = std::type_index(typeid(T));
        if (storages.find(type) == storages.end()) {
            storages[type] = std::make_unique<ComponentStorage<T>>();

            std::cerr << "[INFO] Registrado componente: " << type.name() << "\n";
        }
    }

    // Registra um componente
    template<typename T, typename... Args>
    T& AddComponent(Entity entity, Args&&... args) {   
        try {
            Register<T>();
            auto* storage = GetStorage<T>(); 
            if (!storage)  throw std::runtime_error(std::string("Storage é nullptr para tipo: ") + typeid(T).name()); 

            storage->Add(entity, T(std::forward<Args>(args)...));
            return storage->Get(entity);
        }
        catch (const std::exception& e) {
            std::cerr << "[ERRO] AddComponent em ComponentManager falhou para tipo " << typeid(T).name() << ": " << e.what() << "\n";
            throw; // repassa para o chamador
        }
    }

    template<typename T>
    void RemoveComponent(Entity entity) {
        auto* storage = GetStorage<T>();
        storage->Remove(entity);
    }

    // Checagem compile-time
    template<typename T>
    bool HasComponent(Entity entity) const {
        auto* storage = GetStorage<T>();
        return storage->Has(entity);
    }

    template<typename T>
    T& GetComponent(Entity entity) {
        auto* storage = GetStorage<T>();
        return storage->Get(entity);
    }

    template<typename... Components>
    std::vector<Entity> GetEntitiesWith() {
        std::vector<Entity> result; 
        try {
            auto* first = GetStorage<typename std::tuple_element<0, std::tuple<Components...>>::type>();
            for (Entity e : first->GetEntitySet()) {
                if ((HasComponent<Components>(e) && ...)) 
                    result.push_back(e); 
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[ERRO] Falha ao buscar entidades com componentes: " << e.what() << " (" << __FILE__ << ":" << __LINE__ << ")\n";
        } 
        return result;
    }

    // Checagem runtime
    bool HasComponentType(Entity entity, std::type_index type) const {
        auto it = storages.find(type);
        if (it == storages.end()) return false;
        return it->second->Has(entity);
    }

    // Lista de tipos de componentes de uma entidade
    std::vector<std::type_index> GetComponentTypes(Entity entity) const {
        std::vector<std::type_index> result;
        for (auto& [type, storage] : storages) {
            if (storage->Has(entity))
                result.push_back(type);
        }
        return result;
    }

    // Retorna todos os storages registrados
    //const std::unordered_map<std::type_index, IComponentStorage*>& GetAllStorages() const {
    //    // map temporário só com ponteiros para IComponentStorage
    //    static std::unordered_map<std::type_index, IComponentStorage*> cache;
    //    cache.clear();
    //    for (auto& [type, storage] : storages) {
    //        cache[type] = storage.get();
    //    }
    //    return cache;
    //}

    // Exponha o mapa REAL (sem cache temporário)
    const std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>>&
        GetAllStorages() const {
        return storages;
    }

    template<typename T>
    Entity GetFirstEntityOfType() const {
        auto it = storages.find(std::type_index(typeid(T)));
        if (it == storages.end()) {
            throw std::runtime_error(std::string("Componente não registrado: ") + typeid(T).name());
        }

        auto* storage = static_cast<ComponentStorage<T>*>(it->second.get());
        const auto& entities = storage->GetEntitySet();

        if (entities.empty()) {
            return INVALID_ENTITY;
        }

        return *entities.begin(); // retorna o primeiro (ordem indefinida, mas ok)
    }


private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> storages;

    template<typename T>
    ComponentStorage<T>* GetStorage() const {
        auto it = storages.find(std::type_index(typeid(T)));
        if (it != storages.end()) {
            return static_cast<ComponentStorage<T>*>(it->second.get());
        }
        throw std::runtime_error("Component not registered");
        std::cerr << "[ERRO] Storage não encontrado para tipo: " << typeid(T).name() << "\n";
        //return nullptr;
    }
};





//auto it = storages.find(typeid(Transform));
//if (it != storages.end()) {
//    auto* storage = static_cast<ComponentStorage<Transform>*>(it->second.get());
//    // agora pode usar storage->AddComponent(...), etc
//}
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

    std::unordered_set<Entity>& GetEntitySet() {
        return entities;
    }

private:
    std::unordered_map<Entity, T> components;
    std::unordered_set<Entity> entities;
};



class ComponentManager {
public:
    template<typename T>
    void Register() {
        std::type_index type = std::type_index(typeid(T));
        if (storages.find(type) == storages.end()) {
            storages[type] = std::make_unique<ComponentStorage<T>>();

            std::cerr << "[INFO] Registrado componente: " << type.name() << "\n";
        }
    }

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
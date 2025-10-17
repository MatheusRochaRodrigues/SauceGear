#pragma once

#include <queue>
#include <cstdint>
#include <unordered_set>

// Definição simples de um Entity como um ID numérico
using Entity = uint32_t;
constexpr Entity INVALID_ENTITY = static_cast<Entity>(-1);
constexpr Entity MAX_ENTITIES = 10000;


class EntityManager {
public:
    EntityManager() {
        for (Entity e = 0; e < MAX_ENTITIES; ++e) {
            availableEntities.push(e);
        }
    }

    Entity CreateEntity() {
        if (availableEntities.empty()) return INVALID_ENTITY;
        Entity id = availableEntities.front();
        availableEntities.pop();

        activeEntities.insert(id); // <- adiciona a entidade criada 
        return id;
    }

    //precisa de alem de por no fim da fila, precisa destroer a entidade pra n acumular
    void DestroyEntity(Entity entity) {
        activeEntities.erase(entity);        // <- remove do conjunto
        availableEntities.push(entity);      // <- libera o ID
    }

    // Método para pegar todas entidades ativas
    std::vector<Entity> GetAllEntities() const {
        return std::vector<Entity>(activeEntities.begin(), activeEntities.end()); 
    }

    // EntityManager.h
    bool Exists(Entity e) const {
        return activeEntities.find(e) != activeEntities.end(); // O(1) hash lookup
    }


private:
    std::queue<Entity> availableEntities;
    std::unordered_set<Entity> activeEntities; // entidades vivas/ativas
};

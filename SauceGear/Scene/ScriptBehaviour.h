#pragma once

#include "Entity.h"

class ScriptBehaviour {
public:
    Entity entity;

    virtual void Start() {}
    virtual void Update(float deltaTime) {}
    virtual ~ScriptBehaviour() = default;
};

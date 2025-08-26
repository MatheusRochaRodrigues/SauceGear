#pragma once

#include "Entity.h"

struct ScriptBehaviour {
    Entity entity;

    virtual void Create() {}
    virtual void Update(float dt) {}  // = 0;
    virtual void Destroy() {}

    virtual ~ScriptBehaviour() = default;
};

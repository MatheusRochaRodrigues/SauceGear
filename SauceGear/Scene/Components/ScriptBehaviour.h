// ScriptBehaviour.h
#pragma once
#include "../Entity.h"

class ScriptBehaviour {
public:
    Entity entity;

    virtual void OnCreate() {}
    virtual void OnUpdate(float dt) {}
    virtual void OnDestroy() {}

    virtual ~ScriptBehaviour() = default;
};

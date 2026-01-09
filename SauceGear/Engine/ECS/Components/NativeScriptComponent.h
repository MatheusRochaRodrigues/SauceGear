#pragma once
#include <functional>

#include "../ScriptBehaviour.h"

struct NativeScriptComponent {
    ScriptBehaviour* instance = nullptr;

    std::function<ScriptBehaviour* ()> InstantiateScript;

    void Bind(std::function<ScriptBehaviour* ()> func) {
        InstantiateScript = func;
    }
};

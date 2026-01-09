#pragma once
#include "../Scene/ScriptBehaviour.h"

class MyScript : public ScriptBehaviour {
public:
    void Update(float dt) override {
        // faz algo com entity
    }
};

// Função que exporta o script
extern "C" __declspec(dllexport)
ScriptBehaviour* CreateScript() {
    return new MyScript();
}

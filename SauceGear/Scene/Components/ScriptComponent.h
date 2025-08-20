#pragma once
#include <iostream>
#include "../ScriptBehaviour.h"
#include "../Entity.h"

struct ScriptComponent {
    ScriptBehaviour* (*InstantiateScript)(); // ponteiro de função
    std::unique_ptr<ScriptBehaviour> instance = nullptr;
};
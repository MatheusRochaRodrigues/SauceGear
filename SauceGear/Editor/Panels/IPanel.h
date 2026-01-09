#pragma once
#include "../../Engine/Core/EngineContext.h" 
#include "../../Engine/Scene/SceneECS.h" 

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuizmo.h" 

struct IPanel {
    virtual void Draw(SceneECS& scene) = 0;
    virtual const char* GetName() = 0;
    virtual ~IPanel() = default;
};

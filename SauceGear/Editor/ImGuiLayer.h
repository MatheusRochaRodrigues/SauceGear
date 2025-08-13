#pragma once

//Engine Dependecies
//#include "../Scene/SceneECS.h" 
#include "../Scene/GameScene.h" 
#include "../Platform/Window.h"
 
#define IMGUI_ENABLE_DOCKING // antes de incluir imgui.h
//#include "imgui.h"
//#include "imgui_impl_glfw.h"
//#include "imgui_impl_opengl3.h"
//#include "ImGuizmo.h" 

#include "Panels/IPanelHelper.h"

using Scene = SceneECS; // Apelido dentro da classe 

class ImGuiLayer {
public:
    void Init(GLFWwindow* window);
    void Begin();
    void End();
    void Shutdown();

    void ShowDockspace(); 
    void RenderIPanels(Scene&);
    void config_style();
     
private:
    std::vector<std::shared_ptr<IPanel>> RegisteredPanels;
    void RegisterPanels();
};

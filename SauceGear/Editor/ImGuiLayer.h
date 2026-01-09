#pragma once

//Engine Dependecies
//#include "../Scene/SceneECS.h" 
#include "../Engine/Scene/GameScene.h" 
#include "../Engine/Platform/Window.h"
 
#define IMGUI_ENABLE_DOCKING // antes de incluir imgui.h
//#include "imgui.h"
//#include "imgui_impl_glfw.h"
//#include "imgui_impl_opengl3.h"
//#include "ImGuizmo.h" 

#include "Panels/IPanelHelper.h"
#include "Panels/InspectorPanel.h"

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
     
    void ShowTopBar();
    bool draggingWindow = false; 
     ImVec2 dragStartWindowPos;
     ImVec2 dragStartMousePos;
     double prevMouseX = 0.0, prevMouseY = 0.0;
     ImVec2 clickOffset; // diferença entre mouse e canto da janela ao iniciar drag

private:
    std::vector<std::shared_ptr<IPanel>> RegisteredPanels;
    void RegisterPanels();
};

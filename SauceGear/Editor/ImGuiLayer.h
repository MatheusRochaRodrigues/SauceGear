#pragma once 
#include "../Engine/Scene/GameScene.h" 
#include "../Engine/Platform/Window.h"
 
#define IMGUI_ENABLE_DOCKING // antes de incluir imgui.h 

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
     
    void ShowTopBar();
    bool draggingWindow = false; 
     ImVec2 dragStartWindowPos;
     ImVec2 dragStartMousePos;
     double prevMouseX = 0.0, prevMouseY = 0.0;
     ImVec2 clickOffset; // diferença entre mouse e canto da janela ao iniciar drag

private:
    std::vector<std::shared_ptr<IPanel>> RegisteredPanels;
    void RegisterPanels();

    //Themes 
    void igThemeV3(
        int hue07 = 1, int alt07 = 7, int nav07 = 5,  //Most Important, i choise default values
        int lit01 = 0, int compact01 = 0, int border01 = 1, int shape0123 = 1
    );
    //Themes 2
    void config_style();
};

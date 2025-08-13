#pragma once
 
#include "../../Core/EngineContext.h"  
#include "../SceneECS.h"  
#include "../System.h"
#include "../Components/NativeScriptComponent.h"

class EditorSystem : public System {
public:
    void Init(GLFWwindow* window);  // inicia ImGui
    void Update(float dt) override; // chama ImGui::NewFrame e renderiza os painéis
    void Shutdown();                // limpa ImGui

private:
    void DrawMenuBar();             // barra superior
    void DrawPanels();              // docking + painéis ativos
};

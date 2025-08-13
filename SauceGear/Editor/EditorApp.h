#pragma once 
#include "ImGuiLayer.h"  
#include "../Core/Application.h"
 
//namespace Editor {
class EditorApp {
public:
    EditorApp(Application* app);
    void Run();
private:
    Application* app;
    Window* window;
    Scene* scene;   
};
//}
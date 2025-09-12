#pragma once
#include "ImGuiLayer.h"  

// forward declarations
class Application;
class Window;
class SceneECS;
class GameScene;

class EditorApp {
public:
    EditorApp(Application* app);
    void Run();
private:
    Application* app;
    Window* window;
    Scene* scene;
};

 
//namespace Editor {
//}
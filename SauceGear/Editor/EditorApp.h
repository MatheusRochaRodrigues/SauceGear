#pragma once 

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
    GameScene* scene;
};

 
//namespace Editor {
//}
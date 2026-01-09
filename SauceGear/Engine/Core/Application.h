#pragma once    

class Window;
class GameScene;
class Renderer;
class Time;
class InputSystem;

//#include "../Editor/EditorApp.h" 
//#define GAME_RUNTIME

class Application {
public:  
    int Init();
    int Run();
    void Update();
    void Shutdown();

    Window* window;
    GameScene* scene;            //SceneECS scene;
    Renderer* renderer;
    Time* time;
    InputSystem* input;
};

 
#pragma once


// Apenas forward declarations aqui 
class SceneECS;
class Renderer;
class Window;
class Time;
class InputSystem;
class Camera;   // para o GetMainCamera
//class SceneManager;


struct EngineContext
{
    //SceneECS* scene = nullptr;
    SceneECS* scene = nullptr;
    Renderer* renderer = nullptr;
    Window* window = nullptr;
    Time* time = nullptr;
    InputSystem* input = nullptr;  // sistema de input dedicado
      
    void SetActiveCamera(Camera* cam) {
        mainCamera = cam;
    }
    Camera* mainCamera = nullptr;   // câmera separada
    Camera* GetMainCamera();
     
    //IRenderPipeline* pipeline = nullptr; // apenas ponteiro cru

    //SceneManager* sceneManager;
      
};

// Singleton acessível globalmente (em versão simples)
extern EngineContext* GEngine;


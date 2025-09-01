#pragma once  
//#include "../Scene/Camera.h"
//#include "../Resources/Model.h"
//#include "../IBLMapGenerator.h"
//#include "../SkyboxRenderer.h"
//#include "../Lighting.h"
 
#include "../Platform/Window.h"           //TEMP ATE SEPARA ENGINE E FOR OPCIONAL 
#include "EngineContext.h"                //TEMP ATE SEPARA ENGINE E FOR OPCIONAL 
#include "../Scene/GameScene.h"   
#include "../ECS/Components/CameraComponent.h"   
#include "../ECS/Systems/SystemHelper.h"   
#include "../Graphics/Renderer.h"         //TEMP ATE SEPARA ENGINE E FOR OPCIONAL    
#include "../Core/Time.h"   
#include "../Core/InputSystem.h"


//#include "../Editor/EditorApp.h" 
//#define GAME_RUNTIME

class Application {
public:
    //Application() = default;

    int Init();
    int Run();
    void Update();
    void Shutdown();

    Window window;
    GameScene scene;            //SceneECS scene;
    Renderer renderer;
    Time time;
    InputSystem input;
};
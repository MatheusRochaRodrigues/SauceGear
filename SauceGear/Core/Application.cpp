#include "Application.h" 
#include "../Platform/Window.h"           //TEMP ATE SEPARA ENGINE E FOR OPCIONAL 
#include "EngineContext.h"                //TEMP ATE SEPARA ENGINE E FOR OPCIONAL 
#include "../Scene/GameScene.h"   
#include "../ECS/Components/CameraComponent.h"   
#include "../ECS/Systems/SystemHelper.h"   
#include "../Graphics/Renderer.h"         //TEMP ATE SEPARA ENGINE E FOR OPCIONAL    
#include "../Core/Time.h"   
#include "../Core/InputSystem.h" 

 
int Application::Init() {
    // cria janela GLFW
    window = new Window();
    if (!window->Create("Editor Window", 1600, 900)) return -1;  // Configura contexto OpenGL e glad  

    time = new Time();
    time->Init();

    input = new InputSystem();

    scene = new GameScene();
    renderer = new Renderer();
    renderer->Init(scene);                                     // setup inicial do renderizador
    scene->initECS();

    GEngine = new EngineContext{ scene, renderer, window, time, input };
    GEngine->input->Initialize(GEngine->window);                 // conecta input ao contexto da janela 
    renderer->Initialize();                              // inicializa entidades/sistemas da cena                scene.Init();   
     
    GEngine->renderer->GetShadowShader_Sun = new Shader("Shadows/ShadowMapCasc.vs", "Shadows/ShadowMapCasc.gs", "Shadows/ShadowMapCasc.fs");
    GEngine->renderer->GetShadowShader_Directional = new Shader("Shadows/ShadowMapD.vs", "Shadows/ShadowMapD.fs");
    GEngine->renderer->GetShadowShader_Point = new Shader("Shadows/ShadowMapP.vs", "Shadows/ShadowMapP.gs", "Shadows/ShadowMapP.fs");
     

    //GBuffer
    GEngine->renderer->GetGBufferShader = new Shader("DeferredShading/gBuffer.vs", "DeferredShading/gBuffer.fs");
    //Lighting pos GBuffer
    GEngine->renderer->GetLightingShader = new Shader("DeferredShading/DeferredShading.vs", "DeferredShading/DeferredShading.fs");
    GEngine->renderer->GetSunLightingShader = new Shader("DeferredShading/DeferredShadingSun.vs", "DeferredShading/DeferredShadingSun.fs");


    // Começa com Blinn-Phong
    //renderSystem.SetRenderer(std::make_unique<BlinnPhongRenderer>());
    //renderSystem.Render(scene); 
    // Troca para PBR
    //renderSystem.SetRenderer(std::make_unique<PBRRenderer>());
    //renderSystem.Render(scene);

    stbi_set_flip_vertically_on_load(true);
}

int Application::Run() { 
    // Inicialização básica  
    Init();
      
    while (!glfwWindowShouldClose(window->GetNativeWindow())) {
        renderer->BeginFrame();  // Limpa buffers, etc. 
         
        Update(); 
          
        window->SwapBuffers();
        window->PollEvents();
    } 

    Shutdown();

    return 0;
}

void Application::Update() {
    //DeltaTime
    time->Update();
    float dt = time->GetDeltaTime();
    //controll update
    input->Update();                     //GEngine->input->Update(); 
    //Scene Update
    scene->Update(dt);    // Atualiza todos os sistemas (inclui physics)     // Atualiza ECS
}

void Application::Shutdown() {
    //renderer.Shutdown();
    //scene.Shutdown();
    //window.Destroy();

    delete GEngine;
    glfwTerminate();
}

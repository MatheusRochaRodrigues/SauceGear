#include "Application.h"  
#include "../Core/InputSystem.h"   // for InputSystem
#include "../Core/Time.h"          // for Time
#include "../Graphics/Renderer.h"  // for Renderer
#include "../Platform/Window.h"    // for Window
#include "../Scene/GameScene.h"    // for GameScene
#include "EngineContext.h"         // for EngineContext, GEngine
#include "GLFW/glfw3.h"            // for glfwTerminate, glfwWindowShouldClose
#include "../Graphics/Shader.h"                // for Shader
#include "stb/stb_image.h"         // for stbi_set_flip_vertically_on_load

#include "EditorState.h"
#include "../Assets/AssetDatabase.h"
#include "../Materials/MaterialLibrary.h"

#include "Profiler/Profiler.h" 
 
int Application::Init() {
    // cria janela GLFW
    window = new Window();
    if (!window->Create("GEAR SAUCE", 1600, 900)) return -1;  // Configura contexto OpenGL e glad  

    time = new Time();
    time->Init();

    input = new InputSystem();
     
    ShaderLibrary::Init();                  
    MaterialLibrary::InitMaterials(); 

    scene = new GameScene();
    renderer = new Renderer();
    renderer->Init(scene);                                     // setup inicial do renderizador
    scene->initECS();

    GEngine = new EngineContext{ scene, renderer, window, time, input, new EditorState};
    GEngine->input->Initialize(GEngine->window);                 // conecta input ao contexto da janela 
    renderer->Initialize();                              // inicializa entidades/sistemas da cena                scene.Init();    
        
       
    stbi_set_flip_vertically_on_load(true);
}

int Application::Run() {  
    Init();     // Inicialização básica  
      
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
    Profiler::Get().BeginFrame();

    //DeltaTime
    time->Update();
    float dt = time->GetDeltaTime();
    //controll update
    input->Update();                     //GEngine->input->Update(); 
    //Scene Update
    scene->Update(dt);    // Atualiza todos os sistemas (inclui physics)     // Atualiza ECS

    AssetDatabase::Update();

    Profiler::Get().EndFrame(); // coleta GPU atrasada
}

void Application::Shutdown() {
    //renderer.Shutdown();
    //scene.Shutdown();
    //window.Destroy();

    delete GEngine;
    glfwTerminate();
}

#include "EditorApp.h"

void EditorApp::Run() {
    ImGuiLayer imguiLayer;
    imguiLayer.Init(window->GetNativeWindow());

    while (!window->ShouldClose()) {
        window->PollEvents(); 
        app->Update();                  // atualiza tudo e renderiza cena

        //Tell OpenGL, this is where we start drawing one new frame 
        imguiLayer.Begin();

        imguiLayer.ShowDockspace(); 
        imguiLayer.RenderIPanels(*scene);
          
        // Renders the ImGUI elements
        imguiLayer.End();

        window->SwapBuffers();
    }

    // Deletes all ImGUI instances
    imguiLayer.Shutdown();
    //window.Destroy();
}

EditorApp::EditorApp(Application* app) {
    this->app = app;
    window = &app->window;
    scene  = &app->scene; 
}





//// ImGUI window creation
//ImGui::Begin("My name is window, ImGUI window");
//// Text that appears in the window
//ImGui::Text("Hello there adventurer!");
//// Checkbox that appears in the window
//ImGui::Checkbox("Draw Triangle", &drawTriangle);
//// Slider that appears in the window
//ImGui::SliderFloat("Size", &size, 0.5f, 2.0f);
//// Fancy color editor that appears in the window
//ImGui::ColorEdit4("Color", color);
//// Ends the window
//ImGui::End();

    /*bool drawTriangle = true;
    float size = 1.0f;
    float color[4] = { 0.8f, 0.3f, 0.02f, 1.0f };*/
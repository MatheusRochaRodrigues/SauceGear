#define EDITOR_BUILD
#include <iostream>
#include "EditorApp.h"
#include "../Engine/Core/Application.h"   // <- precisa desse 

int main() { 
    try {
        Application app;
        app.Init();                    // apenas inicializa

        EditorApp editor(&app);       // recebe ponteiro para reuso da app
        editor.Run();                 // editor tem loop próprio (com ImGui)

        app.Shutdown();               // encerra subsistemas no fim 
   
    } catch (const std::exception& e) {
        std::cerr << "[Error Main] " << e.what() << "\n"; 
    } catch (...) {
        std::cerr << "[Error Main2]  dfdsf \n";
    }

    return 0;
}



///////Faça isso só nos pontos de integração, como:
// 
//// EngineContext.cpp
//#ifdef EDITOR_BUILD
//#include "../Editor/EditorApp.h"
//#endif
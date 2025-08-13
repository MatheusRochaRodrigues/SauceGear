#define EDITOR_BUILD
#include "EditorApp.h"

int main() { 
    Application app;
    app.Init();                    // apenas inicializa

    EditorApp editor(&app);       // recebe ponteiro para reuso da app
    editor.Run();                 // editor tem loop próprio (com ImGui)

    app.Shutdown();               // encerra subsistemas no fim
    std::cout << "deu";
    return 0;
}



///////Faça isso só nos pontos de integração, como:
// 
//// EngineContext.cpp
//#ifdef EDITOR_BUILD
//#include "../Editor/EditorApp.h"
//#endif
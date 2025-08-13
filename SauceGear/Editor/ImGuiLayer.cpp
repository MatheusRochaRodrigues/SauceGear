#include "ImGuiLayer.h"
#include "../Core/EngineContext.h" 
#include "../Scene/Components/ComponentsHelper.h" 
#include "../Graphics/Renderer.h" 
 
void ImGuiLayer::Init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    //io.Fonts->AddFontFromFileTTF("fa-solid-900.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesDefault());


    //style.FrameRounding = 0.0f;
    //style.WindowRounding = 0.0f;




    // Zerar os arredondamentos
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;

    //// Opcional: bordas mais finas (deixa mais flat)
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.TabBorderSize = 1.0f;

    ImGui::StyleColorsDark(); // Ou Light()

    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Para arrastar para fora da janela
    // 
    //// Para múltiplas janelas
    //ImGuiStyle& style = ImGui::GetStyle();
    //if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //    style.WindowRounding = 0.0f;
    //    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    //}

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 440");         //"#version 450"
    //ImGui::GetIO().IniFilename = "meu_layout.ini"; // Define o nome do arquivo de layout
    //Se quiser desativar o salvamento, basta fazer:
    //ImGui::GetIO().IniFilename = nullptr; 
     

    //ImGui já salva automaticamente no imgui.ini por padrão. Para salvar manualmente(ex: botão ou menu) : 
    //ImGui::SaveIniSettingsToDisk("layout.ini");
    //E carregar:
    //ImGui::LoadIniSettingsFromDisk("layout.ini"); 




    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

        //style.WindowRounding = 0.0f;
        //style.Colors[ImGuiCol_WindowBg].w = 1.0f;


        //ImVec4 titleBarColor = ImVec4(0.0f, 0.4f, 0.0f, 1.0f);  // Verde

        //ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = titleBarColor;         // Cor da barra de título
        //ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = titleBarColor;   // Cor da barra de título quando ativa
        //ImGui::GetStyle().Colors[ImGuiCol_TitleBgCollapsed] = titleBarColor; // Cor da barra de título quando colapsada

    // Personalizando as cores do tema (vermelho, verde, azul)
    //colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.3f, 0.0f, 1.0f);   // Fundo da janela (verde)
    //colors[ImGuiCol_Button] = ImVec4(0.2f, 0.5f, 0.2f, 1.0f);        // Botões (verde)
    //colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.7f, 0.3f, 1.0f); // Hover nos botões (verde claro)
    //colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.8f, 0.1f, 1.0f);  // Botão pressionado (verde escuro)
    //colors[ImGuiCol_Border] = ImVec4(0.0f, 0.1f, 0.0f, 0.5f);        // Borda (verde escuro)

    //ImGui::GetStyle().WindowPadding = ImVec2(10, 10); // Ajuste de padding
    ImGui::GetStyle().FramePadding = ImVec2(5, 5);   // Ajuste do padding do frame
        //ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Barra de título verde


        ImVec4 dockingPreviewColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Cor verde para o preview de docking
        ImVec4 dockingEmptyBgColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Cor verde para área de docking vazia

        ImGui::GetStyle().Colors[ImGuiCol_DockingPreview] = dockingPreviewColor;
        ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg] = dockingEmptyBgColor;



    //ImVec4 greenColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Cor verde

    // Modificando as cores para o verde
    //style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);   // Cor do fundo das janelas
    //style.Colors[ImGuiCol_TitleBg] = greenColor;                         // Cor de fundo da barra de título
    //style.Colors[ImGuiCol_TitleBgActive] = greenColor;                    // Cor de fundo da barra de título quando ativa
    //style.Colors[ImGuiCol_TitleBgCollapsed] = greenColor;                 // Cor quando a janela está colapsada
    //style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);     // Borda invisível para evitar contornos
    //style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // Sem sombra na borda

    // Ajustando as cores do docking para o verde
    //style.Colors[ImGuiCol_DockingEmptyBg] = greenColor;  // Cor do fundo do docking
    //style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.0f, 1.0f, 0.0f, 0.5f); // Cor do preview do docking

    // Aumentando o tamanho das barras de título (isso afeta todas as janelas)
    //style.WindowTitleAlign = ImVec2(0.5f, 0.5f);    // Centralizando o título
    //style.FramePadding = ImVec2(8, 8);               // Aumentando o padding dentro dos frames
    //style.ItemSpacing = ImVec2(10, 10);              // Aumentando o espaçamento dos itens no layout

    // Aumentando a altura das janelas
    //style.WindowRounding = 5.0f;     // Arredondar cantos das janelas
    //style.ChildRounding = 5.0f;      // Arredondar cantos dos painéis filhos
    //style.FrameRounding = 4.0f;      // Arredondar os frames dentro da janela

    // Modificar a largura das barras de título das janelas
    //style.WindowMinSize = ImVec2(250, 250); // Tamanho mínimo das janelas
    //style.Alpha = 1.0f;  // Definir a opacidade para 100%

    // Alterar o tamanho das fontes (se desejar)
    //ImGui::GetIO().FontGlobalScale = 1.05f;  // Aumentar o tamanho das fontes globalmente

    // Altera o estilo somente para a janela "Hierarchy"
    //ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));  // Verde
    //ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));  // Verde
    //ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));  // Remove a borda

    //ImGui::Begin("Hierarchy");
    //// Seu código para a Hierarchy aqui...
    //ImGui::End();

    //// Restaura o estilo original após a janela "Hierarchy"
    //ImGui::PopStyleColor(3);


    config_style();


    RegisterPanels();
    // Restaura o estilo original
    //ImGui::PopStyleColor();
 
}

void ImGuiLayer::config_style() {

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.04f, 0.04f, 0.04f, 0.99f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.51f, 0.51f, 0.80f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.03f, 0.03f, 0.04f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.60f, 0.10f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.60f, 0.10f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.43f, 0.90f, 0.11f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.21f, 0.22f, 0.23f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.51f, 0.51f, 0.80f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.54f, 0.55f, 0.55f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.38f, 0.51f, 0.51f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.16f, 0.16f, 0.16f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.23f, 0.23f, 0.24f, 0.80f);
    colors[ImGuiCol_Tab] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.13f, 0.78f, 0.07f, 1.00f);
    colors[ImGuiCol_TabDimmed] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.10f, 0.60f, 0.12f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.14f, 0.87f, 0.05f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.30f, 0.60f, 0.10f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.23f, 0.78f, 0.02f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.46f, 0.47f, 0.46f, 0.06f);
    colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.78f, 0.69f, 0.69f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style.WindowRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 3.0f;
    style.PopupRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.ScrollbarSize = 10.0f;
    style.GrabMinSize = 10.0f;
    style.DockingSeparatorSize = 1.0f;
    style.SeparatorTextBorderSize = 2.0f;
}

void ImGuiLayer::RegisterPanels() {
    RegisteredPanels.push_back(std::make_shared<SceneViewPanel>());
    RegisteredPanels.push_back(std::make_shared<HierarchyPanel>());
    RegisteredPanels.push_back(std::make_shared<InspectorPanel>());
    RegisteredPanels.push_back(std::make_shared<FileExplorerPanel>());
}

void ImGuiLayer::RenderIPanels(Scene& scene) {
    // Chamar painéis registrados
    for (auto& panel : RegisteredPanels)
        panel->Draw(scene);
}


void ImGuiLayer::Begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::ShowDockspace() {
    //creating Dock Space Container
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    // setup viewport
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    //style border
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    //add more flags
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    //Cria a janela principal que vai servir como container do sistema de docking.
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(2);
    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

    // Menu no topo
    //if (ImGui::BeginMenuBar()) {
    //    if (ImGui::BeginMenu("Janela")) {
    //        if (ImGui::MenuItem("Hierarquia")) {/* abrirHierarquia = true; */ }
    //        if (ImGui::MenuItem("Inspector")) {/* abrirInspector = true; */ }
    //        if (ImGui::MenuItem("Console")) {/* abrirConsole = true; */ }
    //        ImGui::EndMenu();
    //    }
    //    ImGui::EndMenuBar();
    //}

    ImGui::End();
     



    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Salvar Layout")) {
                ImGui::SaveIniSettingsToDisk("layout.ini");
            }
            if (ImGui::MenuItem("Carregar Layout")) {
                ImGui::LoadIniSettingsFromDisk("layout.ini");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            /*ImGui::MenuItem("Hierarquia", nullptr, &showHierarchy);
            ImGui::MenuItem("Inspetor", nullptr, &showInspector);
            ImGui::MenuItem("Console", nullptr, &showConsole);*/
            //ImGui::MenuItem("Modo Tela Cheia", nullptr, &isSceneFullscreen); // Radio Button para alternar o modo
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    /*if (showHierarchy) {
        ImGui::Begin("Hierarquia", &showHierarchy);
        ImGui::Text("Conteúdo da hierarquia...");
        ImGui::End();
    }*/
}
  


void ImGuiLayer::End() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}





//void DrawPanelWindow(Panel& panel) {
//    ImGui::Begin(panel.name.c_str());
//
//    // Dropdown de tipo no canto
//    if (ImGui::BeginPopup("PanelMenu")) {
//        for (int i = 0; i < (int)PanelType::Assets + 1; i++) {
//            PanelType type = (PanelType)i;
//            if (type != panel.type && !PanelInUse(type)) {
//                if (ImGui::MenuItem(GetPanelTypeName(type))) {
//                    panel.type = type;
//                }
//            }
//        }
//        ImGui::EndPopup();
//    }
//
//    if (ImGui::BeginPopupContextItem("PanelContext"))
//        ImGui::OpenPopup("PanelMenu");
//
//    // Conteúdo do painel
//    switch (panel.type) {
//    case PanelType::Hierarchy: DrawHierarchy(); break;
//    case PanelType::Scene: DrawScene(); break;
//    case PanelType::Inspector: DrawInspector(); break;
//    case PanelType::Assets: DrawAssets(); break;
//    default: break;
//    }
//
//    ImGui::End();
//}
//const char* GetPanelTypeName(PanelType type) {
//    switch (type) {
//    case PanelType::Scene: return "Scene";
//    case PanelType::Hierarchy: return "Hierarchy";
//    case PanelType::Inspector: return "Inspector";
//    case PanelType::Assets: return "Assets";
//    default: return "None";
//    }
//}




//void Render() {
//    ImGui::NewFrame();
//
//    // Altera o estilo somente para a janela "Hierarchy"
//    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));  // Verde
//    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));  // Verde
//    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));  // Remove a borda
//
//    ImGui::Begin("Hierarchy");
//    // Seu código para a Hierarchy aqui...
//    ImGui::End();
//
//    // Restaura o estilo original após a janela "Hierarchy"
//    ImGui::PopStyleColor(3);
//
//    ImGui::Render();
//}
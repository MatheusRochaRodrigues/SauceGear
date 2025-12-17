#include "ImGuiLayer.h"
#include "../Core/EngineContext.h"  
#include "../Graphics/Renderer.h" 
 
void ImGuiLayer::Init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = "GearSauce.init"; // Não salva nem carrega layouts em arquivo

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    // Fonte normal
    //io.Fonts->AddFontFromFileTTF("Assets/Fonts/ProggyTiny.ttf", 14.5f);
    //io.Fonts->AddFontFromFileTTF("fa-solid-900.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesDefault());  
 
    // Carregar fonte padrão (ex: Roboto, Arial, etc.)
    io.Fonts->AddFontFromFileTTF("Assets/Fonts/RobotoSlab-ExtraBold.ttf", 19.5);
     
    // Intervalo da Private Use Area da Material Icons          Range dos ícones (Material Icons ficam nesse bloco Unicode)
    static const ImWchar icons_ranges[] = { ICON_MIN_MI, ICON_MAX_MI, 0 };

    // Config pra mesclar com a fonte padrão
    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;

    // Carregar Material Icons
    io.Fonts->AddFontFromFileTTF("Assets/Fonts/MaterialIcons-Regular.ttf", 18.0f, &config, icons_ranges);

    //ImGui::GetStyle().ScaleAllSizes(1.5f); // aumenta o tamanho global


    //style.FrameRounding = 0.0f;
    //style.WindowRounding = 0.0f; 
    // 
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

    //// Para múltiplas janelas
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Para arrastar para fora da janela
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    } 

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");         //"#version 450" 

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

    ImGui::GetStyle().FramePadding = ImVec2(5, 5);   // Ajuste do padding do frame 
    ImVec4 dockingPreviewColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Cor verde para o preview de docking
    ImVec4 dockingEmptyBgColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Cor verde para área de docking vazia

    ImGui::GetStyle().Colors[ImGuiCol_DockingPreview] = dockingPreviewColor;
    ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg] = dockingEmptyBgColor;
     
    config_style();


    RegisterPanels(); 

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
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
    ImGuiIO& io = ImGui::GetIO();
    const float topBarHeight = 48.0f;

    // --- TopBar ---
    ShowTopBar(); // Fica fixa no topo, fora do DockSpace

    // --- DockSpace abaixo da TopBar ---
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 dockPos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + topBarHeight);
    ImVec2 dockSize = ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - topBarHeight);
    ImGui::SetNextWindowPos(dockPos);
    ImGui::SetNextWindowSize(dockSize);


    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags dockspaceFlags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("DockSpace", nullptr, dockspaceFlags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
    ImGui::End();
     
}

struct WindowState {
    bool maximized = false;
    int prevX, prevY, prevW, prevH;
    bool resizing = false;
};
static WindowState m_WindowState;


void ImGuiLayer::ShowTopBar() {
    GLFWwindow* m_Window = GEngine->window->GetNativeWindow();
    ImGuiIO& io = ImGui::GetIO();
    const float topBarHeight = 48.0f;
    const float buttonSize = 28.0f;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, topBarHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 6));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::Begin("TopBar", nullptr, flags);

    // --- LOGO ---
    static ImTextureID logoTex = guiLoadTexture("assets/icons/Ilustração2.png");
    if (logoTex)
        ImGui::Image(logoTex, ImVec2(topBarHeight - 12, topBarHeight - 12), ImVec2(0, 1), ImVec2(1, 0));
    else ImGui::TextUnformatted("L");

    // --- TÍTULO CENTRAL ---
    const char* title = "Gear Sauce";
    ImVec2 textSize = ImGui::CalcTextSize(title);
    float titleX = (io.DisplaySize.x - textSize.x) * 0.5f;  //centraliza o texto horizontalmente.    //Calula largura do texto e coloca o cursor de texto no X centralizado
    ImGui::SetCursorPos(ImVec2(titleX, 6.0f));
    ImGui::TextUnformatted(title);

    // --- MENUS E BOTÕES ---
    /*ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - 200, 6.0f));
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save Layout")) ImGui::SaveIniSettingsToDisk("layout.ini");
        if (ImGui::MenuItem("Load Layout")) ImGui::LoadIniSettingsFromDisk("layout.ini");
        ImGui::EndMenu();
    }
    ImGui::SameLine();*/

    // Botões janela
    ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - (buttonSize * 3 + 25), 6.0f));
    if (ImGui::Button("_", ImVec2(buttonSize, buttonSize)))
        glfwIconifyWindow(m_Window);
    ImGui::SameLine();
    if (ImGui::Button("[ ]", ImVec2(buttonSize, buttonSize))) {
        if (!m_WindowState.maximized) {
            //glfwGetWindowPos(m_Window, &m_WindowState.prevX, &m_WindowState.prevY); //glfwGetWindowSize(m_Window, &m_WindowState.prevW, &m_WindowState.prevH);
            glfwMaximizeWindow(m_Window);
            m_WindowState.maximized = true;
        }
        else {
            glfwRestoreWindow(m_Window);
            //glfwSetWindowPos(m_Window, m_WindowState.prevX, m_WindowState.prevY); //glfwSetWindowSize(m_Window, m_WindowState.prevW, m_WindowState.prevH);
            m_WindowState.maximized = false;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("X", ImVec2(buttonSize, buttonSize)))
        glfwSetWindowShouldClose(m_Window, GLFW_TRUE);

    // --- INVISIBLE DRAG AREA ---
    ImGui::SetCursorPos(ImVec2(0, 0));

    ImGui::InvisibleButton("DragArea", ImVec2(io.DisplaySize.x, topBarHeight));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
        int wx, wy;
        glfwGetWindowPos(m_Window, &wx, &wy);

        double mx, my;
        glfwGetCursorPos(m_Window, &mx, &my);
        //aboslute == mouse global position
        double absMouseX = wx + mx;
        double absMouseY = wy + my;

        if (!draggingWindow) {
            clickOffset = ImVec2(absMouseX - wx, absMouseY - wy);
            draggingWindow = true;
        }

        int newX = (int)(absMouseX - clickOffset.x);
        int newY = (int)(absMouseY - clickOffset.y);
        glfwSetWindowPos(m_Window, newX, newY);
    }
    else {
        draggingWindow = false;
    }
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        if (!m_WindowState.maximized) { 
            glfwMaximizeWindow(m_Window);   //glfwGetWindowPos(m_Window, &m_WindowState.prevX, &m_WindowState.prevY); glfwGetWindowSize(m_Window, &m_WindowState.prevW, &m_WindowState.prevH);
            m_WindowState.maximized = true;
        } else {
            glfwRestoreWindow(m_Window);   //glfwSetWindowPos(m_Window, m_WindowState.prevX, m_WindowState.prevY); glfwSetWindowSize(m_Window, m_WindowState.prevW, m_WindowState.prevH);
            m_WindowState.maximized = false;
        }
    }
        //bool hovering = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow);
        //if (hovering && ImGui::IsMouseDragging(0) && !ImGui::IsItemActive()) {
    /*ImGui::InvisibleButton("DragArea", ImVec2(io.DisplaySize.x, topBarHeight));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
        ImVec2 delta = io.MouseDelta;
        int x, y;
        glfwGetWindowPos(m_Window, &x, &y);
        glfwSetWindowPos(m_Window, x + (int)delta.x, y + (int)delta.y);
    }*/


    ImGui::End();
    ImGui::PopStyleVar(3);


    // --- RESIZE BORDAS (direita/baixo/canto) ---
    const float BORDER = 6.0f;
    int winX, winY, winW, winH;
    glfwGetWindowPos(m_Window, &winX, &winY);
    glfwGetWindowSize(m_Window, &winW, &winH);

    ImVec2 mousePos = io.MousePos;
    bool resizing = false;

    // canto inferior direito
    if (mousePos.x >= winW - BORDER && mousePos.y >= winH - BORDER) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
        if (ImGui::IsMouseDown(0)) {
            glfwSetWindowSize(m_Window, (int)mousePos.x, (int)mousePos.y);
            resizing = true;
        }
    }
    // direita
    else if (mousePos.x >= winW - BORDER) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        if (ImGui::IsMouseDown(0)) {
            glfwSetWindowSize(m_Window, (int)mousePos.x, winH);
            resizing = true;
        }
    }
    // inferior
    else if (mousePos.y >= winH - BORDER) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        if (ImGui::IsMouseDown(0)) {
            glfwSetWindowSize(m_Window, winW, (int)mousePos.y);
            resizing = true;
        }
    }

    m_WindowState.resizing = resizing;
}



void ImGuiLayer::End() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    ImGuiIO& io = ImGui::GetIO();
    // Renderiza as viewports extras (janela GLFW real)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void ImGuiLayer::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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
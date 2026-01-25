#include "ImGuiLayer.h"

#include "../Engine/Core/EngineContext.h"  
#include "../Engine/Graphics/Renderer.h" 

#include "Panels/SettingsPanel.h" 
#include "Panels/SceneViewPanel.h"
#include "Panels/FileExplorer/FileExplorerPanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h" 
 
void ImGuiLayer::Init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = "GearSauce.init"; // Não salva nem carrega layouts em arquivo

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors; 
 
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
    // Zerar os arredondamentos
    /*style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;*/

    //// Opcional: bordas mais finas (deixa mais flat)
    /*style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.TabBorderSize = 1.0f;*/

    ImGui::StyleColorsDark(); // Ou Light()

    //// Para múltiplas janelas
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Para arrastar para fora da janela
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    } 

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");         //"#version 450" 
     
     
    //config_style();
    igThemeV3();


    RegisterPanels();  
}


void ImGuiLayer::RegisterPanels() {
    RegisteredPanels.push_back(std::make_shared<SceneViewPanel>());
    RegisteredPanels.push_back(std::make_shared<HierarchyPanel>());
    RegisteredPanels.push_back(std::make_shared<InspectorPanel>());
    RegisteredPanels.push_back(std::make_shared<FileExplorerPanel>());
    RegisteredPanels.push_back(std::make_shared<SettingsPanel>()); 
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




//igThemeV3
void ImGuiLayer::igThemeV3(
    int hue07, int alt07, int nav07,  //Most Important, i choise default values
    int lit01, int compact01, int border01, int shape0123
) {     
    bool rounded = shape0123 == 2;

    // V3 style from ImThemes
    ImGuiStyle& style = ImGui::GetStyle();

    const float _8 = compact01 ? 4 : 8;
    const float _4 = compact01 ? 2 : 4;
    const float _2 = compact01 ? 0.5 : 1;

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.3f;

    style.WindowPadding = ImVec2(4, _8);
    style.FramePadding = ImVec2(4, _4);
    style.ItemSpacing = ImVec2(_8, _2 + _2);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.IndentSpacing = 16;
    style.ScrollbarSize = compact01 ? 12 : 18;
    style.GrabMinSize = compact01 ? 16 : 20;

    style.WindowBorderSize = border01;
    style.ChildBorderSize = border01;
    style.PopupBorderSize = border01;
    style.FrameBorderSize = 0;

    style.WindowRounding = 4;
    style.ChildRounding = 6;
    style.FrameRounding = shape0123 == 0 ? 0 : shape0123 == 1 ? 4 : 12;
    style.PopupRounding = 4;
    style.ScrollbarRounding = rounded * 8 + 4;
    style.GrabRounding = style.FrameRounding;

    style.TabBorderSize = 0;
    style.TabBarBorderSize = 2;
    style.TabBarOverlineSize = 2;
    style.TabCloseButtonMinWidthSelected = -1; // -1:always visible, 0:visible when hovered, >0:visible when hovered if minimum width
    style.TabCloseButtonMinWidthUnselected = -1;
    style.TabRounding = rounded;

    style.CellPadding = ImVec2(8.0f, 4.0f);

    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Right;

    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.5f, 0.5f);
    style.SeparatorTextAlign.x = 1.00f;
    style.SeparatorTextBorderSize = 1;
    style.SeparatorTextPadding = ImVec2(0, 0);

    style.WindowMinSize = ImVec2(32.0f, 16.0f);
    style.ColumnsMinSpacing = 6.0f;

    // diamond sliders
    style.CircleTessellationMaxError = shape0123 == 3 ? 4.00f : 0.30f;

    auto lit = [&](ImVec4 hi) {
        float h, s, v; ImGui::ColorConvertRGBtoHSV(hi.x, hi.y, hi.z, h, s, v);
        ImVec4 lit = ImColor::HSV(h, s * 0.80, v * 1.00, hi.w).Value;
        return lit;
        };
    auto dim = [&](ImVec4 hi) {
        float h, s, v; ImGui::ColorConvertRGBtoHSV(hi.x, hi.y, hi.z, h, s, v);
        ImVec4 dim = ImColor::HSV(h, s, lit01 ? v * 0.65 : v * 0.65, hi.w).Value;
        if (hi.z > hi.x && hi.z > hi.y) return ImVec4(dim.x, dim.y, hi.z, dim.w);
        return dim;
        };

    const ImVec4 cyan = ImVec4(000 / 255.f, 192 / 255.f, 255 / 255.f, 1.00f);
    const ImVec4 red = ImVec4(230 / 255.f, 000 / 255.f, 000 / 255.f, 1.00f);
    const ImVec4 yellow = ImVec4(240 / 255.f, 210 / 255.f, 000 / 255.f, 1.00f);
    const ImVec4 orange = ImVec4(255 / 255.f, 144 / 255.f, 000 / 255.f, 1.00f);
    const ImVec4 lime = ImVec4(192 / 255.f, 255 / 255.f, 000 / 255.f, 1.00f);
    const ImVec4 aqua = ImVec4(000 / 255.f, 255 / 255.f, 192 / 255.f, 1.00f);
    const ImVec4 magenta = ImVec4(255 / 255.f, 000 / 255.f, 88 / 255.f, 1.00f);
    const ImVec4 purple = ImVec4(192 / 255.f, 000 / 255.f, 255 / 255.f, 1.00f);

    ImVec4 alt = cyan;
    /**/ if (alt07 == 0 || alt07 == 'C') alt = cyan;
    else if (alt07 == 1 || alt07 == 'R') alt = red;
    else if (alt07 == 2 || alt07 == 'Y') alt = yellow;
    else if (alt07 == 3 || alt07 == 'O') alt = orange;
    else if (alt07 == 4 || alt07 == 'L') alt = lime;
    else if (alt07 == 5 || alt07 == 'A') alt = aqua;
    else if (alt07 == 6 || alt07 == 'M') alt = magenta;
    else if (alt07 == 7 || alt07 == 'P') alt = purple;
    if (lit01) alt = dim(alt);

    ImVec4 hi = cyan, lo = dim(cyan);
    /**/ if (hue07 == 0 || hue07 == 'C') lo = dim(hi = cyan);
    else if (hue07 == 1 || hue07 == 'R') lo = dim(hi = red);
    else if (hue07 == 2 || hue07 == 'Y') lo = dim(hi = yellow);
    else if (hue07 == 3 || hue07 == 'O') lo = dim(hi = orange);
    else if (hue07 == 4 || hue07 == 'L') lo = dim(hi = lime);
    else if (hue07 == 5 || hue07 == 'A') lo = dim(hi = aqua);
    else if (hue07 == 6 || hue07 == 'M') lo = dim(hi = magenta);
    else if (hue07 == 7 || hue07 == 'P') lo = dim(hi = purple);
    //    if( lit01 ) { ImVec4 tmp = hi; hi = lo; lo = lit(tmp); }

    ImVec4 nav = orange;
    /**/ if (nav07 == 0 || nav07 == 'C') nav = cyan;
    else if (nav07 == 1 || nav07 == 'R') nav = red;
    else if (nav07 == 2 || nav07 == 'Y') nav = yellow;
    else if (nav07 == 3 || nav07 == 'O') nav = orange;
    else if (nav07 == 4 || nav07 == 'L') nav = lime;
    else if (nav07 == 5 || nav07 == 'A') nav = aqua;
    else if (nav07 == 6 || nav07 == 'M') nav = magenta;
    else if (nav07 == 7 || nav07 == 'P') nav = purple;
    if (lit01) nav = dim(nav);

    const ImVec4
        link = ImVec4(0.26f, 0.59f, 0.98f, 1.00f),
        grey0 = ImVec4(0.04f, 0.05f, 0.07f, 1.00f),
        grey1 = ImVec4(0.08f, 0.09f, 0.11f, 1.00f),
        grey2 = ImVec4(0.10f, 0.11f, 0.13f, 1.00f),
        grey3 = ImVec4(0.12f, 0.13f, 0.15f, 1.00f),
        grey4 = ImVec4(0.16f, 0.17f, 0.19f, 1.00f),
        grey5 = ImVec4(0.18f, 0.19f, 0.21f, 1.00f);

#define Luma(v,a) ImVec4((v)/100.f,(v)/100.f,(v)/100.f,(a)/100.f)

    style.Colors[ImGuiCol_Text] = Luma(100, 100);
    style.Colors[ImGuiCol_TextDisabled] = Luma(39, 100);
    style.Colors[ImGuiCol_WindowBg] = grey1;
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = grey1;
    style.Colors[ImGuiCol_Border] = grey4;
    style.Colors[ImGuiCol_BorderShadow] = grey1;
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.13f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = grey4;
    style.Colors[ImGuiCol_FrameBgActive] = grey4;
    style.Colors[ImGuiCol_TitleBg] = grey0;
    style.Colors[ImGuiCol_TitleBgActive] = grey0;
    style.Colors[ImGuiCol_TitleBgCollapsed] = grey1;
    style.Colors[ImGuiCol_MenuBarBg] = grey2;
    style.Colors[ImGuiCol_ScrollbarBg] = grey0;
    style.Colors[ImGuiCol_ScrollbarGrab] = grey3;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = lo;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = hi;
    style.Colors[ImGuiCol_CheckMark] = alt;
    style.Colors[ImGuiCol_SliderGrab] = lo;
    style.Colors[ImGuiCol_SliderGrabActive] = hi;
    style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = lo;
    style.Colors[ImGuiCol_ButtonActive] = grey5;
    style.Colors[ImGuiCol_Header] = grey3;
    style.Colors[ImGuiCol_HeaderHovered] = lo;
    style.Colors[ImGuiCol_HeaderActive] = hi;
    style.Colors[ImGuiCol_Separator] = ImVec4(0.13f, 0.15f, 0.19f, 1.00f);
    style.Colors[ImGuiCol_SeparatorHovered] = lo;
    style.Colors[ImGuiCol_SeparatorActive] = hi;
    style.Colors[ImGuiCol_ResizeGrip] = Luma(15, 100);
    style.Colors[ImGuiCol_ResizeGripHovered] = lo;
    style.Colors[ImGuiCol_ResizeGripActive] = hi;
    style.Colors[ImGuiCol_InputTextCursor] = Luma(100, 100);
    style.Colors[ImGuiCol_TabHovered] = grey3;
    style.Colors[ImGuiCol_Tab] = grey1;
    style.Colors[ImGuiCol_TabSelected] = grey3;
    style.Colors[ImGuiCol_TabSelectedOverline] = hi;
    style.Colors[ImGuiCol_TabDimmed] = grey1;
    style.Colors[ImGuiCol_TabDimmedSelected] = grey1;
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = lo;
    style.Colors[ImGuiCol_DockingPreview] = grey1;
    style.Colors[ImGuiCol_DockingEmptyBg] = Luma(20, 100);
    style.Colors[ImGuiCol_PlotLines] = grey5;
    style.Colors[ImGuiCol_PlotLinesHovered] = lo;
    style.Colors[ImGuiCol_PlotHistogram] = grey5;
    style.Colors[ImGuiCol_PlotHistogramHovered] = lo;
    style.Colors[ImGuiCol_TableHeaderBg] = grey0;
    style.Colors[ImGuiCol_TableBorderStrong] = grey0;
    style.Colors[ImGuiCol_TableBorderLight] = grey0;
    style.Colors[ImGuiCol_TableRowBg] = grey3;
    style.Colors[ImGuiCol_TableRowBgAlt] = grey2;
    style.Colors[ImGuiCol_TextLink] = link;
    style.Colors[ImGuiCol_TextSelectedBg] = Luma(39, 100);
    style.Colors[ImGuiCol_TreeLines] = Luma(39, 100);
    style.Colors[ImGuiCol_DragDropTarget] = nav;
    style.Colors[ImGuiCol_NavCursor] = nav;
    style.Colors[ImGuiCol_NavWindowingHighlight] = lo;
    style.Colors[ImGuiCol_NavWindowingDimBg] = Luma(0, 63);
    style.Colors[ImGuiCol_ModalWindowDimBg] = Luma(0, 63);

    if (lit01) {
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            float H, S, V;
            ImVec4& col = style.Colors[i];
            ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);
            if (S < 0.5) V = 1.0 - V, S *= 0.15;
            ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
        }
    }
     
}






void ImGuiLayer::config_style() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Base Colors
    ImVec4 bgColor = ImVec4(0.10f, 0.105f, 0.11f, 1.00f);
    ImVec4 lightBgColor = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    ImVec4 panelColor = ImVec4(0.17f, 0.18f, 0.19f, 1.00f);
    ImVec4 panelHoverColor = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);
    ImVec4 panelActiveColor = ImVec4(0.23f, 0.26f, 0.29f, 1.00f);
    ImVec4 textColor = ImVec4(0.86f, 0.87f, 0.88f, 1.00f);
    ImVec4 textDisabledColor = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    ImVec4 borderColor = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);

    // Text
    colors[ImGuiCol_Text] = textColor;
    colors[ImGuiCol_TextDisabled] = textDisabledColor;

    // Windows
    colors[ImGuiCol_WindowBg] = bgColor;
    colors[ImGuiCol_ChildBg] = bgColor;
    colors[ImGuiCol_PopupBg] = bgColor;
    colors[ImGuiCol_Border] = borderColor;
    colors[ImGuiCol_BorderShadow] = borderColor;

    // Headers
    colors[ImGuiCol_Header] = panelColor;
    colors[ImGuiCol_HeaderHovered] = panelHoverColor;
    colors[ImGuiCol_HeaderActive] = panelActiveColor;

    // Buttons
    colors[ImGuiCol_Button] = panelColor;
    colors[ImGuiCol_ButtonHovered] = panelHoverColor;
    colors[ImGuiCol_ButtonActive] = panelActiveColor;

    // Frame BG
    colors[ImGuiCol_FrameBg] = lightBgColor;
    colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
    colors[ImGuiCol_FrameBgActive] = panelActiveColor;

    // Tabs
    colors[ImGuiCol_Tab] = panelColor;
    colors[ImGuiCol_TabHovered] = panelHoverColor;
    colors[ImGuiCol_TabActive] = panelActiveColor;
    colors[ImGuiCol_TabUnfocused] = panelColor;
    colors[ImGuiCol_TabUnfocusedActive] = panelHoverColor;

    // Title
    colors[ImGuiCol_TitleBg] = bgColor;
    colors[ImGuiCol_TitleBgActive] = bgColor;
    colors[ImGuiCol_TitleBgCollapsed] = bgColor;

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = bgColor;
    colors[ImGuiCol_ScrollbarGrab] = panelColor;
    colors[ImGuiCol_ScrollbarGrabHovered] = panelHoverColor;
    colors[ImGuiCol_ScrollbarGrabActive] = panelActiveColor;

    // Checkmark
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    // Slider
    colors[ImGuiCol_SliderGrab] = panelHoverColor;
    colors[ImGuiCol_SliderGrabActive] = panelActiveColor;

    // Resize Grip
    colors[ImGuiCol_ResizeGrip] = panelColor;
    colors[ImGuiCol_ResizeGripHovered] = panelHoverColor;
    colors[ImGuiCol_ResizeGripActive] = panelActiveColor;

    // Separator
    colors[ImGuiCol_Separator] = borderColor;
    colors[ImGuiCol_SeparatorHovered] = panelHoverColor;
    colors[ImGuiCol_SeparatorActive] = panelActiveColor;

    // Plot
    colors[ImGuiCol_PlotLines] = textColor;
    colors[ImGuiCol_PlotLinesHovered] = panelActiveColor;
    colors[ImGuiCol_PlotHistogram] = textColor;
    colors[ImGuiCol_PlotHistogramHovered] = panelActiveColor;

    // Text Selected BG
    colors[ImGuiCol_TextSelectedBg] = panelActiveColor;

    // Modal Window Dim Bg
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.105f, 0.11f, 0.5f);

    // Tables
    colors[ImGuiCol_TableHeaderBg] = panelColor;
    colors[ImGuiCol_TableBorderStrong] = borderColor;
    colors[ImGuiCol_TableBorderLight] = borderColor;
    colors[ImGuiCol_TableRowBg] = bgColor;
    colors[ImGuiCol_TableRowBgAlt] = lightBgColor;

    // Styles
    style.FrameBorderSize = 1.0f;
    style.FrameRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.ScrollbarSize = 12.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabMinSize = 7.0f;
    style.GrabRounding = 2.0f;
    style.TabBorderSize = 1.0f;
    style.TabRounding = 2.0f;

    // Reduced Padding and Spacing
    style.WindowPadding = ImVec2(5.0f, 5.0f);
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);

    // Font Scaling
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 0.95f;

    io.Fonts->AddFontDefault();
    float baseFontSize = 18.0f;
    float iconFontSize = baseFontSize * 2.0f / 3.0f;

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;

    /*io.Fonts->AddFontFromFileTTF(
        (std::string(RESOURCE_DIR) + "/fonts/" + FONT_ICON_FILE_NAME_FA).c_str(), iconFontSize,
        &icons_config, icons_ranges);*/
}
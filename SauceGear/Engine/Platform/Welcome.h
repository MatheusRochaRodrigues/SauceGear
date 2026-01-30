#pragma once

namespace SG::Welcome {

    void ClearConsole();
    void PrintSauceGear();
    void TypeText(const char* text, int delayMs = 18);
    void Load(const char* name);

    void SetColor(int win, int ansi);
    void SetColor(int winColor, const char* ansi);
    void ResetColor();


    static bool s_unicode;
    static bool s_ansi;
}




//#pragma once
//#include <iostream>
//#include <thread>
//#include <chrono>
//
//#include <GLFW/glfw3.h>
//#include <glad/glad.h>
//
//#ifdef _WIN32
//#include <windows.h>
//#endif
//
//// =======================================================
//// Console
//// =======================================================
//void ClearConsole() {
//#ifdef _WIN32
//    system("cls");
//#else
//    system("clear");
//#endif
//}
//
//void SetColor(int win, int ansi) {
//#ifdef _WIN32
//    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), win);
//#else
//    std::cout << "\033[" << ansi << "m";
//#endif
//}
//
//void ResetColor() {
//#ifdef _WIN32
//    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
//#else
//    std::cout << "\033[0m";
//#endif
//}
//
//void TypeText(const std::string& t, int d = 18) {
//    for (char c : t) {
//        std::cout << c << std::flush;
//        std::this_thread::sleep_for(std::chrono::milliseconds(d));
//    }
//}
//
//// =======================================================
//// SAUCE GEAR ASCII
//// =======================================================
//void PrintSauceGear() {
//
//    // SAUCE
//    SetColor(12, 38); // laranja / vermelho
//    std::cout <<
//        R"(███████╗ █████╗ ██╗   ██╗ ██████╗███████╗
//██╔════╝██╔══██╗██║   ██║██╔════╝██╔════╝
//███████╗███████║██║   ██║██║     █████╗
//╚════██║██╔══██║██║   ██║██║     ██╔══╝
//███████║██║  ██║╚██████╔╝╚██████╗███████╗)";
//
//    std::cout << "\n";
//
//    // GEAR
//    SetColor(7, 37); // cinza
//    std::cout <<
//        R"( ██████╗ ███████╗ █████╗ ██████╗
//██╔════╝ ██╔════╝██╔══██╗██╔══██╗
//██║  ███╗█████╗  ███████║██████╔╝
//██║   ██║██╔══╝  ██╔══██║██╔══██╗
//╚██████╔╝███████╗██║  ██║██║  ██║
// ╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝)";
//
//    ResetColor();
//    std::cout << "\n\n";
//}
//
//// =======================================================
//// Loading
//// =======================================================
//void Load(const std::string& name) {
//    SetColor(11, 36); // ciano
//    std::cout << "[BOOT] ";
//    ResetColor();
//
//    std::cout << name << " ";
//
//    SetColor(10, 32); // verde
//    std::cout << "[";
//    for (int i = 0; i < 24; i++) {
//        std::cout << "#";
//        std::cout.flush();
//        std::this_thread::sleep_for(std::chrono::milliseconds(20));
//    }
//    std::cout << "] OK\n";
//    ResetColor();
//}
//
//// =======================================================
//// Main
//// =======================================================
//int main() {
//
//    ClearConsole();
//    PrintSauceGear();
//
//    SetColor(11, 36);
//    TypeText("Initializing SAUCE GEAR Engine\n\n");
//
//    // GLFW
//    if (!glfwInit()) {
//        SetColor(12, 31);
//        std::cout << "GLFW init failed\n";
//        return -1;
//    }
//    Load("GLFW");
//
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//    GLFWwindow* window = glfwCreateWindow(1280, 720, "SAUCE GEAR", nullptr, nullptr);
//    glfwMakeContextCurrent(window);
//
//    // GLAD
//    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
//        SetColor(12, 31);
//        std::cout << "OpenGL loader failed\n";
//        return -1;
//    }
//    Load("OpenGL");
//
//    // GPU INFO
//    std::cout << "\n";
//    SetColor(11, 36);
//    std::cout << "GPU      : " << glGetString(GL_RENDERER) << "\n";
//    std::cout << "Vendor   : " << glGetString(GL_VENDOR) << "\n";
//    std::cout << "OpenGL   : " << glGetString(GL_VERSION) << "\n";
//    ResetColor();
//
//    std::cout << "\n";
//    SetColor(10, 32);
//    std::cout << "SAUCE GEAR READY\n";
//    ResetColor();
//
//    // Loop
//    while (!glfwWindowShouldClose(window)) {
//        glfwPollEvents();
//        glfwSwapBuffers(window);
//    }
//
//    glfwDestroyWindow(window);
//    glfwTerminate();
//    return 0;
//}

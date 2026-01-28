#include "Welcome.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <windows.h>
#endif

namespace SG::Welcome {

#ifdef _WIN32
    void EnableANSI()
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
#endif



    void InitConsoleUTF8()
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif
    }

    void ClearConsole() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void SetColor(int winColor, const char* ansi)
    {
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), winColor);
#else
        std::cout << ansi;
#endif
    }

    void SetColor(int win, int ansi) {
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), win);
#else
        std::cout << "\033[" << ansi << "m";
#endif
    }


    void ResetColor() {
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#else
        std::cout << "\033[0m";
#endif
    }

    void TypeText(const char* t, int d) {
        while (*t) {
            std::cout << *t++ << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(d));
        }
    }

    void PrintSauceGear()
    {
        InitConsoleUTF8();
        EnableANSI();




        SetColor(12, "\033[38;5;196m");
        std::cout <<
            R"(███████╗ █████╗ ██╗   ██╗ ██████╗███████╗
██╔════╝██╔══██╗██║   ██║██╔════╝██╔════╝
███████╗███████║██║   ██║██║     █████╗
╚════██║██╔══██║██║   ██║██║     ██╔══╝
███████║██║  ██║╚██████╔╝╚██████╗███████╗)";
        ResetColor();

        std::cout << "\n";

        SetColor(7, "\033[38;5;250m");
        std::cout <<
            R"( ██████╗ ███████╗ █████╗ ██████╗
██╔════╝ ██╔════╝██╔══██╗██╔══██╗
██║  ███╗█████╗  ███████║██████╔╝
██║   ██║██╔══╝  ██╔══██║██╔══██╗
╚██████╔╝███████╗██║  ██║██║  ██║
 ╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝)";
        ResetColor();

        std::cout << "\n\n";
    }


    void Load(const char* name) {
        SetColor(11, 36);
        std::cout << "[BOOT] ";
        ResetColor();

        std::cout << name << " ";

        SetColor(10, 32);
        std::cout << "[########################] OK\n";
        ResetColor();
    }

}

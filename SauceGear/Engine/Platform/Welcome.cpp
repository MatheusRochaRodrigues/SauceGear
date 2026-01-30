#include "Welcome.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fcntl.h>
#include <io.h>

#ifdef _WIN32
#include <windows.h>
#endif



namespace SG::Welcome {

#ifdef _WIN32
    static int g_oldStdoutMode = -1;
    static int g_oldStderrMode = -1;
#endif


#ifdef _WIN32 

    void SetupConsole()
    {
        // Habilita ANSI
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        GetConsoleMode(hOut, &mode);
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, mode);

        // SALVA o modo antigo
        g_oldStdoutMode = _setmode(_fileno(stdout), _O_U16TEXT);
        g_oldStderrMode = _setmode(_fileno(stderr), _O_U16TEXT);
    }

#endif


#ifdef _WIN32
    void RestoreConsole()
    {
        if (g_oldStdoutMode != -1)
            _setmode(_fileno(stdout), g_oldStdoutMode);

        if (g_oldStderrMode != -1)
            _setmode(_fileno(stderr), g_oldStderrMode);
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
        SetupConsole();


        SetColor(12, "\033[38;5;196m");
        std::wcout <<
            L" ██████╗ ███████╗ █████╗ ██████╗\n"
            L"██╔════╝ ██╔════╝██╔══██╗██╔══██╗\n"
            L"██║  ███╗█████╗  ███████║██████╔╝\n"
            L"██║   ██║██╔══╝  ██╔══██║██╔══██╗\n"
            L"╚██████╔╝███████╗██║  ██║██║  ██║\n"
            L" ╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝\n";

        ResetColor();

        std::wcout << "\n";

        SetColor(12, "\033[38;5;196m");
        std::wcout <<
            L"███████╗ █████╗ ██╗   ██╗ ██████╗███████╗\n"
            L"██╔════╝██╔══██╗██║   ██║██╔════╝██╔════╝\n"
            L"███████╗███████║██║   ██║██║     █████╗\n"
            L"╚════██║██╔══██║██║   ██║██║     ██╔══╝\n"
            L"███████║██║  ██║╚██████╔╝╚██████╗███████╗\n";

        ResetColor();
         
        std::wcout << "\n\n";


#ifdef _WIN32
        RestoreConsole(); // 🔥 VOLTA AO NORMAL
#endif


        std::cout << "Log normal ASCII\n\n";

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

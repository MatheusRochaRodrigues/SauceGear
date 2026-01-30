#pragma once 
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Welcome.h"

class Window {
public:
    bool Create(const char* title, int width, int height) {
        m_title = title;
        m_width = width;
        m_height = height;
        m_window = nullptr;
        return InitializeGL();
    };

    bool InitializeGL() {
        SG::Welcome::ClearConsole();  SG::Welcome::PrintSauceGear();
        SG::Welcome::SetColor(11, 36);  SG::Welcome::TypeText("Initializing SAUCE GEAR Engine\n\n");

        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        SG::Welcome::Load("GLFW");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        //glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


    #ifdef EDITOR_BUILD
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);     //new - to heed bar window white
    #endif
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);     //new - to heed bar window white

        m_window = glfwCreateWindow(m_width, m_height, m_title, nullptr, nullptr);
        if (!m_window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_window);


        /*glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);*/

        //glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            SG::Welcome::SetColor(12, 31);
            std::cerr << "Failed to initialize GLAD" << std::endl;
            std::cout << "OpenGL loader failed\n";
            return false;
        }
        //-----------------------------------------------------------
            //EnableGLDebug();    // Ative apenas para Debug, pois é pesado
        //-----------------------------------------------------------

        SG::Welcome::Load("OpenGL");

        // GPU INFO
        std::cout << "\n";
        SG::Welcome::SetColor(11, 36);
        std::cout << "GPU      : " << glGetString(GL_RENDERER) << "\n";
        std::cout << "Vendor   : " << glGetString(GL_VENDOR) << "\n";
        std::cout << "OpenGL   : " << glGetString(GL_VERSION) << "\n";
        SG::Welcome::ResetColor();

        std::cout << "\n";
        SG::Welcome::SetColor(10, 32);
        std::cout << "SAUCE GEAR READY\n";
        SG::Welcome::ResetColor();



        glViewport(0, 0, m_width, m_height);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);


        return true;
    } 

    GLFWwindow* GetNativeWindow() { return m_window; }      //cuidado

    void SwapBuffers() { glfwSwapBuffers(m_window); }
    void PollEvents() { glfwPollEvents(); }
    bool ShouldClose() const { return glfwWindowShouldClose(m_window); }
    
    float GetAspectRatio() const {
        return static_cast<float>(m_width) / static_cast<float>(m_height);
    }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    void SetWindowViewport0() { glViewport(0, 0, m_width, m_height); }

    int m_width, m_height;
private:
    const char* m_title;
    GLFWwindow* m_window;



    static void APIENTRY GLDebugCallback(
        GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length,
        const GLchar* message, const void* userParam)
    {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;

        std::cerr << "🔥 GL DEBUG\n";
        std::cerr << "Message: " << message << "\n";
        std::cerr << "Type: " << type << "  Severity: " << severity << "\n\n";
    }


    void EnableGLDebug()
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallback, nullptr);


        /*Se quiser pegar só erros graves, filtre:
        glDebugMessageControl(
            GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DEBUG_SEVERITY_NOTIFICATION,
            0, nullptr,
            GL_FALSE
        ); 
        */
    }

};

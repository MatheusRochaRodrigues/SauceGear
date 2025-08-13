#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

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
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        //glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }

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
};

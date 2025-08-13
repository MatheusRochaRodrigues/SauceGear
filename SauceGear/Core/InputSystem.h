#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include "KeyCodes.h" 

class Window;


class InputSystem {
public:
    void Initialize(Window* _window) ;

    void Update(); // chamada a cada frame

    bool IsKeyDown(int key);
    bool IsKeyPressed(int key);
    bool IsKeyUp(int key);

    bool IsMouseDown(int button);
    bool IsMousePressed(int button);
    bool IsMouseReleased(int button);

    glm::vec2 GetMousePosition() const;
    glm::vec2 GetMouseDelta();

private:
    GLFWwindow* window;

    std::unordered_map<int, bool> keysDown;
    std::unordered_map<int, bool> keysPressed;
    std::unordered_map<int, bool> keysReleased;

    std::unordered_map<int, bool> mouseDown;
    std::unordered_map<int, bool> mousePressed;
    std::unordered_map<int, bool> mouseReleased;

    //glm::vec2 mousePosition = { 0, 0 };
    double lastMouseX = 0, lastMouseY = 0;
    double mouseX = 0, mouseY = 0;

    //friend class EngineContext;
};

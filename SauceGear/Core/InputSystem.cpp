#include "InputSystem.h"
#include "../Platform/Window.h"

void InputSystem::Initialize(Window* _window) {
    window = _window->GetNativeWindow();

    /*glfwSetCursorPosCallback(m_Window->GetNativeWindow(), MouseCallback);
    glfwSetScrollCallback(m_Window->GetNativeWindow(), ScrollCallback);
    glfwSetInputMode(m_Window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED); */
};

void InputSystem::Update() {
    keysPressed.clear();
    keysReleased.clear();
    mousePressed.clear();
    mouseReleased.clear();

    for (int key = 0; key < 350; key++) {
        bool isDown = glfwGetKey(window, key) == GLFW_PRESS;
        if (isDown && !keysDown[key]) keysPressed[key] = true;
        if (!isDown && keysDown[key]) keysReleased[key] = true;
        keysDown[key] = isDown;
    }

    for (int button = 0; button < 8; button++) {
        bool isDown = glfwGetMouseButton(window, button) == GLFW_PRESS;
        if (isDown && !mouseDown[button]) mousePressed[button] = true;
        if (!isDown && mouseDown[button]) mouseReleased[button] = true;
        mouseDown[button] = isDown;
    }

    lastMouseX = mouseX;
    lastMouseY = mouseY;

    glfwGetCursorPos(window, &mouseX, &mouseY);


    // Posição do mouse
    /*double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mousePosition = glm::vec2((float)xpos, (float)ypos);*/
}

bool InputSystem::IsKeyDown(int key) { return keysDown[key]; }
bool InputSystem::IsKeyPressed(int key) { return keysPressed[key]; }
bool InputSystem::IsKeyUp(int key) { return keysReleased[key]; }

bool InputSystem::IsMouseDown(int btn) { return mouseDown[btn]; }
bool InputSystem::IsMousePressed(int btn) { return mousePressed[btn]; }
bool InputSystem::IsMouseReleased(int btn) { return mouseReleased[btn]; }

glm::vec2 InputSystem::GetMousePosition() const {
    return glm::vec2(mouseX, mouseY);
}


glm::vec2 InputSystem::GetMouseDelta() {
    return glm::vec2(mouseX - lastMouseX, mouseY - lastMouseY);
}






















//#include "InputSystem.h"
//#include "../Platform/Window.h"
//#include <GLFW/glfw3.h>
//
//void InputSystem::Initialize(Window* window) {
//    m_Window = window;
//
//    /*glfwSetCursorPosCallback(m_Window->GetNativeWindow(), MouseCallback);
//    glfwSetScrollCallback(m_Window->GetNativeWindow(), ScrollCallback);
//    glfwSetInputMode(m_Window->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);*/
//}
//
//void InputSystem::Update() {
//    prevKeyState = currKeyState;
//    prevMouseState = currMouseState;
//
//    // Teclado
//    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key)
//        currKeyState[key] = glfwGetKey(m_Window->GetNativeWindow(), key) == GLFW_PRESS;
//
//    // Mouse
//    for (int btn = GLFW_MOUSE_BUTTON_1; btn <= GLFW_MOUSE_BUTTON_LAST; ++btn)
//        currMouseState[btn] = glfwGetMouseButton(m_Window->GetNativeWindow(), btn) == GLFW_PRESS;
//
//    // Posição do mouse
//    double xpos, ypos;
//    glfwGetCursorPos(m_Window->GetNativeWindow(), &xpos, &ypos);
//    mousePosition = glm::vec2((float)xpos, (float)ypos);
//}
//
//bool InputSystem::IsKeyPressed(int key) const {
//    auto it = currKeyState.find(key);
//    return it != currKeyState.end() && it->second;
//}
//
//bool InputSystem::IsKeyDown(int key) const {
//    return currKeyState[key] && !prevKeyState[key];
//}
//
//bool InputSystem::IsKeyReleased(int key) const {
//    return !currKeyState[key] && prevKeyState[key];
//}
//
//bool InputSystem::IsMousePressed(int button) const {
//    return currMouseState[button];
//}
//
//glm::vec2 InputSystem::GetMousePosition() const {
//    return mousePosition;
//}

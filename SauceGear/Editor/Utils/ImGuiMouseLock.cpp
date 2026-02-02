#include "ImGuiMouseLock.h"
#include "../../../Engine/Core/EngineContext.h"
#include "../../../Engine/Platform/Window.h"
#include <imgui.h>
#include <GLFW/glfw3.h> 

void ImGuiLockMouseWhileActive()
{
    return;
    GLFWwindow* window = GEngine->window->GetNativeWindow();
    static bool locked = false;
    static ImVec2 lockPos;

    ImGuiIO& io = ImGui::GetIO();

    if (ImGui::IsItemActivated()) {
        lockPos = io.MousePos;
        locked = true;
    }

    if (ImGui::IsItemActive() && locked) {
        glfwSetCursorPos(window, lockPos.x, lockPos.y);
        ImGui::SetMouseCursor(ImGuiMouseCursor_None); // opcional (fica lindo)
    }

    if (ImGui::IsItemDeactivated()) {
        locked = false;
    }
}

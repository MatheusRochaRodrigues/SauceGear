#pragma once
#include <glm/glm.hpp>
#include "EngineContext.h"
#include "InputSystem.h"

//API global (Input::GetKey, etc.)
class Input {
public:
    // Tecla pressionada agora
    static bool GetKey(int key) {
        return GEngine && GEngine->input ? GEngine->input->IsKeyDown(key) : false;
    }

    // Tecla foi pressionada neste frame
    static bool GetKeyDown(int key) {
        return GEngine && GEngine->input ? GEngine->input->IsKeyPressed(key) : false;
    }

    // Tecla foi solta neste frame
    static bool GetKeyUp(int key) {
        return GEngine && GEngine->input ? GEngine->input->IsKeyUp(key) : false;
    }

    // Mouse
    static glm::vec2 GetMousePosition() {
        return (GEngine && GEngine->input) ? GEngine->input->GetMousePosition() : glm::vec2(0);
    }

    static float GetAxis(const std::string& axisName) {
        // Exemplo: "Horizontal" -> A/D ou seta esq/dir
        if (axisName == "Horizontal") {
            return (float)GetKey(KEY_D) - (float)GetKey(KEY_A); 
        }
        else if (axisName == "Vertical") {
            return (float)GetKey(KEY_W) - (float)GetKey(KEY_S); 
        }
        return 0.0f;
    }

    static bool GetMouseButton(int button) {
        return GEngine && GEngine->input ? GEngine->input->IsMouseDown(button) : false;
    }

    static bool GetMouseButtonDown(int button) {
        return GEngine && GEngine->input ? GEngine->input->IsMousePressed(button) : false;
    }

    static bool GetMouseButtonUp(int button) {
        return GEngine && GEngine->input ? GEngine->input->IsMouseReleased(button) : false;
    }

    static glm::vec2 GetMouseDelta() {
        return GEngine && GEngine->input ? GEngine->input->GetMouseDelta() : glm::vec2(0);
    }

};

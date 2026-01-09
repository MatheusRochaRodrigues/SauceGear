#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "../Scene/Entity.h"

struct EditorState {
    // ---------- viewport ----------
    glm::vec2 sceneViewportPos{ 0.0f };
    glm::vec2 sceneViewportSize{ 0.0f };
    bool sceneViewportHovered = false;
    bool sceneViewportFocused = false;

    // ---------- mouse ----------
    glm::vec2 mouseScreen{ 0.0f };
    glm::vec2 mouseViewport{ 0.0f };

    // ---------- selection ----------
    std::vector<Entity> selection;
    Entity primarySelection = INVALID_ENTITY;

    // ---------- gizmo ----------
    bool gizmoUsing = false;
    bool gizmoOver = false;

    enum class GizmoMode { Translate, Rotate, Scale };
    enum class GizmoSpace { World, Local };

    GizmoMode gizmoMode = GizmoMode::Translate;
    GizmoSpace gizmoSpace = GizmoSpace::World;

    bool snapEnabled = false;
    glm::vec3 snapTranslate{ 1.0f };
    glm::vec3 snapRotate{ 15.0f };
    glm::vec3 snapScale{ 0.1f };

    // ---------- editor flags ----------
    bool wantsPick = false;
    bool wantsBoxSelect = false;
    bool blockEngineInput = false;

    // ---------- debug ----------
    bool drawAABB = true;
};

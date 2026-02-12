#pragma once
#include "IPanel.h"
#include <imgui.h>
#include <algorithm>
#include "../../Engine/Core/Profiler/FrameMetrics.h"

struct MetricsPanel : IPanel {

    static ImVec4 FPSColor(float fps) {
        if (fps >= 90.0f) return { 0.2f, 0.9f, 0.3f, 1.0f };   // verde
        if (fps >= 60.0f) return { 0.9f, 0.8f, 0.2f, 1.0f };   // amarelo
        return { 0.9f, 0.3f, 0.3f, 1.0f };                     // vermelho
    }

    void Draw(SceneECS&) override {
        ImGui::Begin("Metrics");

        const auto& fm = g_FrameMetrics.Read();

        // ================= FPS =================
        if (ImGui::BeginTable("FPS_Table", 2,
            ImGuiTableFlags_SizingFixedFit |
            ImGuiTableFlags_NoBordersInBody))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("FPS");

            ImGui::TableNextColumn();
            ImGui::TextColored(FPSColor(fm.fpsAvg), "%.1f", fm.fpsAvg);

            ImGui::EndTable();
        }

        ImGui::Separator();
        float fpsNorm = std::min(fm.fpsAvg / 120.0f, 1.0f);
        ImGui::ProgressBar(fpsNorm, ImVec2(-1, 20));
        ImGui::Separator();

        // ================= FRAME TIME =================
        ImGui::Spacing();

        if (ImGui::BeginTable("FrameTime_Table", 2,
            ImGuiTableFlags_SizingFixedFit |
            ImGuiTableFlags_NoBordersInBody))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Frame Time");

            ImGui::TableNextColumn();
            ImGui::Text("%.2f ms (avg %.2f)", fm.frameMs, fm.frameAvg);

            ImGui::EndTable();
        }

        // Sparkline
        /*
        static float history[120] = {};
        static int offset = 0;

        history[offset] = fm.frameMs;
        offset = (offset + 1) % IM_ARRAYSIZE(history);

        ImGui::PlotLines(
            "##frametime",
            history,
            IM_ARRAYSIZE(history),
            offset,
            nullptr,
            0.0f,
            33.0f,
            ImVec2(0, 50)
        );
        */

        // ================= RENDER STATS =================
        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::BeginTable("RenderStats_Table", 2,
            ImGuiTableFlags_SizingFixedFit |
            ImGuiTableFlags_NoBordersInBody))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Draw Calls");
            ImGui::TableNextColumn();
            ImGui::Text("%u", fm.drawCalls);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Triangles");
            ImGui::TableNextColumn();
            ImGui::Text("%u", fm.triangles);

            ImGui::EndTable();
        }

        ImGui::End();
    }

    const char* GetName() override { return "Metrics"; }
};
















 
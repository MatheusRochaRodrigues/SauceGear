#pragma once
#include <imgui.h>
#include <vector>
#include <algorithm>
#include "../../Engine/Core/Profiler/Profiler.h"

struct ProfilerGraphWindow
{
    bool open = true;
    const ProfileSample* sample = nullptr;

    static float AutoMax(const std::vector<float>& v)
    {
        if (v.empty()) return 1.0f;
        float m = *std::max_element(v.begin(), v.end());
        return m < 0.1f ? 0.1f : m * 1.25f;
    }

    void Draw(const char* name)
    {
        ImGui::SetNextWindowSize(ImVec2(520, 360), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin(
            name,
            &open,
            ImGuiWindowFlags_NoDocking   // 🔥 docking OFF
        ))
        {
            ImGui::End();
            return;
        }

        const ProfileSample& s = *sample;


        // -------- Metrics --------
        float cpuMax = AutoMax(s.cpuHistory);
        float gpuMax = AutoMax(s.gpuHistory);

        ImGui::Text("CPU avg: %.2f ms", s.cpuAvg);
        ImGui::SameLine(); 
        ImGui::Text("\t CPU max: %.2f ms", cpuMax);  

        ImGui::Separator();

        // ================= Metrics =================

        /*ImGui::Text("CPU avg: %.2f ms", s.cpuAvg);
        ImGui::SameLine();
        ImGui::Text("GPU avg: %.2f ms", s.gpuAvg);

        float cpuMax = AutoMax(s.cpuHistory);
        float gpuMax = AutoMax(s.gpuHistory);
        float sharedMax = std::max(cpuMax, gpuMax);

        ImGui::Text("Scale max: %.2f ms", sharedMax);
        ImGui::Separator();*/

        // ================= Layout =================

        ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.y < 200.0f)
            avail.y = 200.0f;

        const float graphHeight = (avail.y - 20.0f) * 0.5f;
        const ImVec2 graphSize(avail.x, graphHeight);

        // ================= CPU Graph =================

        //ImGui::Text("CPU history");

        DrawGrid(graphSize);

        ImGui::PushStyleColor(ImGuiCol_PlotLines, { 0.30f, 0.60f, 1.00f, 1.00f });
        ImGui::PlotLines(
            "CPU##cpu",
            s.cpuHistory.data(),
            (int)s.cpuHistory.size(),
            0,
            nullptr,
            0.0f,
            cpuMax,
            graphSize
        );
        ImGui::PopStyleColor();

        ImGui::Spacing();

        // ================= GPU Graph =================

        ImGui::Text("GPU avg: %.2f ms", s.gpuAvg);
        ImGui::SameLine();
        ImGui::Text("\t GPU max: %.2f ms", gpuMax);

        ImGui::Separator();
        //ImGui::Text("GPU history");

        DrawGrid(graphSize);

        ImGui::PushStyleColor(ImGuiCol_PlotLines, { 1.00f, 0.55f, 0.20f, 1.00f });
        ImGui::PlotLines(
            "GPU##gpu",
            s.gpuHistory.data(),
            (int)s.gpuHistory.size(),
            0,
            nullptr,
            0.0f,
            gpuMax,
            graphSize
        );
        ImGui::PopStyleColor();

        ImGui::End();
    }

    // ================= Grid =================

    static void DrawGrid(const ImVec2& size)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();

        constexpr int gridX = 6;
        constexpr int gridY = 4;

        for (int i = 0; i <= gridX; ++i)
        {
            float x = p.x + size.x * i / gridX;
            dl->AddLine(
                ImVec2(x, p.y),
                ImVec2(x, p.y + size.y),
                IM_COL32(80, 80, 80, 90)
            );
        }

        for (int i = 0; i <= gridY; ++i)
        {
            float y = p.y + size.y * i / gridY;
            dl->AddLine(
                ImVec2(p.x, y),
                ImVec2(p.x + size.x, y),
                IM_COL32(80, 80, 80, 90)
            );
        }
    }
};

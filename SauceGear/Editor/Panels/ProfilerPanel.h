#pragma once
#include "IPanel.h"
#include "../../Engine/Core/Profiler/Profiler.h"
#include "ProfilerGraphWindow.h"
#include <imgui.h>
#include <vector>
#include <algorithm>
#include <unordered_map>

struct ProfilerPanel : IPanel {

    // ================= Target FPS =================

    static constexpr float TargetsMs[] = {
        16.66f,
        8.33f,
        6.94f
    };

    static constexpr const char* TargetNames[] = {
        "60 FPS", "120 FPS", "144 FPS"
    };

    // ================= Helpers =================

    static ImVec4 ColorFromPct(float pct) {
        if (pct < 0.20f) return { 0.20f, 0.85f, 0.30f, 1.0f };
        if (pct < 0.40f) return { 0.90f, 0.75f, 0.20f, 1.0f };
        return { 0.90f, 0.30f, 0.30f, 1.0f };
    }

    static float Clamp01(float v) {
        return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    }

    // ================= Graph windows =================

    std::unordered_map<std::string, ProfilerGraphWindow> graphWindows;

    // ================= UI =================

    void Draw(SceneECS&) override {
        ImGui::Begin("Profiler");

        static int targetIdx = 0;
        ImGui::Combo("Target", &targetIdx, TargetNames, IM_ARRAYSIZE(TargetNames));
        const float budgetMs = TargetsMs[targetIdx];

        const auto& samples = Profiler::Get().GetSamples();

        float frameCpu = 0.0f;
        float frameGpu = 0.0f;

        for (auto& [_, s] : samples) {
            frameCpu += s.cpuAvg;
            frameGpu += s.gpuAvg;
        }

        const float frameMs = std::max(frameCpu, frameGpu);
        const float framePct = frameMs / budgetMs;

        ImGui::Separator();
        ImGui::Text("Frame: %.2f ms / %.2f ms (%.0f%%)", frameMs, budgetMs, framePct * 100.0f);

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ColorFromPct(framePct));
        ImGui::ProgressBar(Clamp01(framePct), ImVec2(-1.0f, 20.0f));
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();

        // ---------------- Sort samples ----------------

        struct Row {
            const char* name;
            const ProfileSample* s;
        };

        std::vector<Row> rows;
        for (auto& [name, s] : samples)
            rows.push_back({ name.c_str(), &s });

        std::sort(rows.begin(), rows.end(),
            [](const Row& a, const Row& b) {
                return (a.s->cpuAvg + a.s->gpuAvg) >
                    (b.s->cpuAvg + b.s->gpuAvg);
            });

        // ================= TABLE =================

        if (ImGui::BeginTable("ProfilerTable", 7,
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersInnerV |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_ScrollY))
        {
            DrawTableHeader();

            for (const Row& r : rows)
                DrawRow(*r.s, r.name, frameMs, budgetMs);

            ImGui::EndTable();
        }

        ImGui::End();

        // -------- Draw graph windows --------
        for (auto it = graphWindows.begin(); it != graphWindows.end(); )
        {
            if (!it->second.open) {
                it = graphWindows.erase(it);
                continue;
            }

            it->second.Draw(it->first.c_str());
            ++it;
        }
    }

    void DrawTableHeader() {
        ImGui::TableSetupColumn(" Section");
        ImGui::TableSetupColumn("CPU (ms)", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableSetupColumn("GPU (ms)", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableSetupColumn("%", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableSetupColumn("Frame");
        ImGui::TableSetupColumn("Target");
        ImGui::TableSetupColumn("Calls", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableHeadersRow();
    }

    void DrawRow(
        const ProfileSample& s,
        const char* name,
        float frameMs,
        float budgetMs)
    {
        float totalMs = s.cpuAvg + s.gpuAvg;
        float pctFrame = frameMs > 0 ? totalMs / frameMs : 0.0f;
        float pctTarget = budgetMs > 0 ? totalMs / budgetMs : 0.0f;

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        if (ImGui::Selectable(name))
        {
            auto& w = graphWindows[name];
            w.open = true;
            w.sample = &s;
        }

        ImGui::TableNextColumn(); ImGui::Text("%.2f", s.cpuAvg);
        ImGui::TableNextColumn(); ImGui::Text("%.2f", s.gpuAvg);
        ImGui::TableNextColumn(); ImGui::Text("%.0f%%", pctTarget * 100.0f);

        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ColorFromPct(pctFrame));
        ImGui::ProgressBar(Clamp01(pctFrame), ImVec2(-1.0f, 5.0f));
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ColorFromPct(pctTarget));
        ImGui::ProgressBar(Clamp01(pctTarget), ImVec2(-1.0f, 5.0f));
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();
        ImGui::Text("%u", s.hits);
    }

    const char* GetName() override { return "Profiler"; }
};

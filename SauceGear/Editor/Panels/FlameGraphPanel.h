//#pragma once
//#include "IPanel.h"
//#include "../../Engine/Core/Profiler/Profiler.h"
//#include <imgui.h>
// 
//
//struct FlameGraphPanel : IPanel {
//
//    void Draw(SceneECS&) override {
//        ImGui::Begin("Flame Graph");
//
//        const ProfileNode& root = Profiler::Get().GetFrameRoot();
//        if (root.children.empty()) {
//            ImGui::TextDisabled("No data");
//            ImGui::End();
//            return;
//        }
//
//        float totalCpu = 0.0f;
//        for (auto& c : root.children)
//            totalCpu += (float)c.cpuMs;
//
//        float width = ImGui::GetContentRegionAvail().x;
//        float x = ImGui::GetCursorScreenPos().x;
//        float y = ImGui::GetCursorScreenPos().y;
//
//        for (auto& c : root.children) {
//            DrawNode(c, totalCpu, x, y, width);
//            x += (float)(c.cpuMs / totalCpu) * width;
//        }
//
//        ImGui::Dummy(ImVec2(width, 220));
//        ImGui::End();
//    }
//
//    void DrawNode(
//        const ProfileNode& n,
//        float total,
//        float x,
//        float y,
//        float width
//    ) {
//        float w = (float)(n.cpuMs / total) * width;
//        if (w < 2.0f) return;
//
//        ImDrawList* dl = ImGui::GetWindowDrawList();
//
//        ImU32 cpuColor = IM_COL32(220, 120, 60, 255);
//        ImU32 gpuColor = IM_COL32(60, 160, 220, 255);
//
//        // CPU
//        dl->AddRectFilled({ x, y }, { x + w, y + 18 }, cpuColor, 2);
//
//        // GPU overlay
//        if (n.gpuMs > 0.0f && n.cpuMs > 0.0f) {
//            float gw = (float)(n.gpuMs / n.cpuMs) * w;
//            gw = std::min(gw, w);
//
//            dl->AddRectFilled(
//                { x, y + 18 },
//                { x + gw, y + 34 },
//                gpuColor,
//                2
//            );
//        }
//
//
//        dl->AddText({ x + 4, y + 2 }, IM_COL32_WHITE, n.name.c_str());
//
//        float childX = x;
//        for (auto& c : n.children) {
//            DrawNode(c, total, childX, y + 36, width);
//            childX += (float)(c.cpuMs / total) * width;
//        }
//    }
//
//    const char* GetName() override { return "FlameGraph"; }
//};

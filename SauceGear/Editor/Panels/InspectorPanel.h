#pragma once
#include <imgui.h>

#include "IPanel.h"
#include "../Utils/InspectorDrawer.h"
#include "../../Engine/Scene/SceneECS.h"
#include "../ImGui/Fonts/IconsFontAwesome5.h"
#include "../../Engine/ECS/Components/MeshRenderer.h"
#include "../Panels/DrawMeshRendererInspector.h"


static void DrawMeshRendererInspector(MeshRenderer& renderer) {

    ImGui::Text("Mesh");
    ImGui::SameLine(120);
    ImGui::TextDisabled(
        renderer.mesh && renderer.mesh->mesh
        ? renderer.mesh->mesh->name.c_str()
        : "None"
    );

    ImGui::Separator();
    
    ImGui::Text("Materials");
    for (size_t i = 0; i < renderer.materials.size(); ++i) {
        auto& mat = renderer.materials[i];

        ImGui::PushID((int)i);

        ImGui::Text("Slot %d", (int)i);
        ImGui::SameLine(120);

        if (mat)
            ImGui::Button(mat->asset->name.c_str(), ImVec2(-1, 0));
        else
            ImGui::Button("None", ImVec2(-1, 0));

        ImGui::PopID();
    }
}
 

// UI helpers (Unity style columns) 
static void BeginRow(const char* label) {
    ImGui::TextUnformatted(label);
    ImGui::SameLine(140);
    ImGui::SetNextItemWidth(-1);
}

static void DrawMaterialBlock(MaterialInstance& inst) { 
    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth;

    std::string header = ICON_FA_PAINT_BRUSH;
    header += " ";
    header += inst.asset->name;

    if (ImGui::TreeNodeEx(inst.asset.get(), flags, "%s", header.c_str())) {

        //ImGui::TextDisabled("Shader");
        //ImGui::SameLine(120);
        BeginRow("Shader");

        ImGui::Text(inst.asset->base->shader->name.c_str());

        ImGui::Separator();

        MaterialInspector::Draw(inst);

        ImGui::TreePop();
    }
}


struct InspectorPanel : IPanel {
    void Draw(SceneECS& scene) override {
        ImGui::Begin("Inspector");

        Entity selected = scene.GetSelectedEntity();
        if (selected == INVALID_ENTITY) {
            ImGui::TextDisabled("No entity selected");
            ImGui::End();
            return;
        }

        for (auto* typeInfo : scene.GetComponentTypes(selected)) {
            void* compPtr = scene.GetComponentRaw(selected, *typeInfo);
            if (!compPtr) continue;

            // ===== HEADER ESTILIZADO ==============================================
            ImGuiTreeNodeFlags nodeFlags =
                //ImGuiTreeNodeFlags_DefaultOpen |      //Começa aberto
                ImGuiTreeNodeFlags_Framed |             //Desenha como um painel
                ImGuiTreeNodeFlags_AllowItemOverlap |   //Permite botão por cima
                ImGuiTreeNodeFlags_SpanAvailWidth;      //Ocupa largura total

            // Altura maior para o header
            float headerHeight = 28.0f; // pode ajustar
            ImVec2 framePadding(8, (headerHeight - ImGui::GetFontSize()) * 0.5f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);

            // Ícone + nome do componente
            std::string header = ICON_FA_CUBE;
            header += " ";
            header += typeInfo->name;
             
            ImGui::PushID(typeInfo);          // escopo único
            bool open = ImGui::TreeNodeEx("##header", nodeFlags, "%s", header.c_str());  //ImGui::TreeNodeEx((void*)typeInfo, nodeFlags, "%s", header.c_str());
            // -------------------------------------------------------------------------

            // ===== BOTÃO X ==========================================================
            if (typeInfo->removable) {
                float buttonSize = ImGui::GetFontSize() * 1.2f;
                //Retorna o retângulo do último item desenhado  -   Aqui: o header do componente.
                ImVec2 lastItemRectMin = ImGui::GetItemRectMin();
                ImVec2 lastItemRectMax = ImGui::GetItemRectMax();

                ImGui::SetCursorScreenPos(ImVec2(
                    lastItemRectMax.x - buttonSize - 6, // 6px margin da borda
                    lastItemRectMin.y + (headerHeight - buttonSize) * 0.5f
                ));

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
                if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
                    scene.RemoveComponent(selected, *typeInfo);
                    if (open) ImGui::TreePop();
                    ImGui::PopStyleVar(); // botão
                    ImGui::PopStyleVar(); // header
                    ImGui::PopID();
                    continue;
                }
                ImGui::PopStyleVar(); // botão
            }
            // -------------------------------------------------------------------------

            // ===== PROPRIEDADES ======================================================
            if (open) {
                ImGui::Indent( );   //10

                // propriedades normais
                for (auto& field : typeInfo->fields)  
                    InspectorDrawer::DrawProperty(field, compPtr, typeInfo); 

                // Extenção manual do MeshRenderer
                if (typeInfo->typeIndex == typeid(MeshRenderer)) {
                    ImGui::Separator();
                    DrawMeshRendererInspector( *static_cast<MeshRenderer*>(compPtr)  );
                }

                ImGui::Unindent( ); //10
                ImGui::TreePop();
            }
            // -------------------------------------------------------------------------
            //Teste
            /*ImVec2 pos;
            if (ImGui::CollapsingHeader("Transform")) {
                ImGui::DragFloat3("Position", &pos.x);
                ImGui::DragFloat3("Rotation", &pos.y);
            }*/

            ImGui::PopID();
            ImGui::PopStyleVar(); // header
            ImGui::Separator();   // linha separadora
        }

        //AddComponent
        //if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddComponentPopup"); 
        ButtonAddComponent();

        if (ImGui::BeginPopup("AddComponentPopup")) {

            ImGui::SetNextItemWidth(220.0f);

            ImGui::BeginChild("ComponentList",
                ImVec2(220, 260),   // largura e altura
                true,
                ImGuiWindowFlags_NoScrollbar
            );

            for (auto& [name, ti] : ReflectionRegistry::Get().GetAll()) {

                if (scene.HasComponentType(selected, ti.typeIndex)) continue; 
                if (!ti.Add) continue; 
                if (!scene.CanAddComponentType(ti.typeIndex)) continue;

                if (ImGui::Selectable(ti.name.c_str(), false)) {
                    ti.Add(&scene, selected);
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndChild();
            ImGui::EndPopup();
        }
         

        // --- MATERIALS (Unity style) ---
        if (scene.HasComponent<MeshRenderer>(selected)) {
            auto& mr = scene.GetComponent<MeshRenderer>(selected);

            for (auto& mat : mr.materials) {
                if (!mat) continue;

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                DrawMaterialBlock(*mat);
            }
        }


        ImGui::End();
    }

    void ButtonAddComponent() {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float width = ImGui::GetContentRegionAvail().x;
        float height = 30.0f;

        // --- Cores (tema vermelho/preto) ---
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.42f, 0.12f, 0.12f, 1.0f)); // quase preto
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.88f, 0.18f, 0.18f, 1.0f)); // hover sutil
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.10f, 0.10f, 1.0f)); // vermelho escuro

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.92f, 0.92f, 1.0f));

        // --- Estilo ---
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));

        if (ImGui::Button(ICON_FA_PLUS "  Add Component", ImVec2(width, height))) {
            ImGui::OpenPopup("AddComponentPopup");
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
    }


    const char* GetName() override { return "Inspector"; }
};



/*
Fluxo mental da construção do ImGui

Begin Window
 ├─ TreeNode (Component)
 │   ├─ Button X
 │   ├─ Indent
 │   │   ├─ Properties
 │   │   ├─ Separator
 │   │   └─ Custom Inspector
 │   ├─ Unindent
 │   └─ TreePop
 └─ End Window

*/


/* Centralizar usando o tamanho real do texto (mais preciso)
const char* label = ICON_FA_PLUS "  Add Component";

ImVec2 textSize = ImGui::CalcTextSize(label);
float paddingX = ImGui::GetStyle().FramePadding.x * 2.0f;
float buttonWidth = textSize.x + paddingX;

float avail = ImGui::GetContentRegionAvail().x;
ImGui::SetCursorPosX((avail - buttonWidth) * 0.5f);

ImGui::Button(label);

*/


 
/* Centralizar vertical e horizontal (layout de editor)
ImVec2 buttonSize(200, 30);
ImVec2 avail = ImGui::GetContentRegionAvail();

ImGui::SetCursorPos({
    (avail.x - buttonSize.x) * 0.5f,
    (avail.y - buttonSize.y) * 0.5f
});

ImGui::Button("Add Component", buttonSize);

*/




/*
void ButtonAddComponent() {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    float width = ImGui::GetContentRegionAvail().x;
    float height = 32.0f;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.55f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.65f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.45f, 0.25f, 1.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

    if (ImGui::Button(ICON_FA_PLUS " Add Component", ImVec2(width, height))) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

}
*/
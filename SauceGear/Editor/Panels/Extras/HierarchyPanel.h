#pragma once
#include "IPanel.h"
#include "../ECS/Components/ComponentsHelper.h" 
#include "../Graphics/Renderer.h"  
#include "../Icons/MaterialIcons.h"  
#include <fstream>
#include <filesystem>

struct HierarchyPanel : IPanel {


    void DrawHierarchyPopup(const std::filesystem::path& currentDirectory) {
        static std::filesystem::path clipboardPath;
        static bool hasClipboard = false;

        if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight)) {

            // ===== Categoria: Import / Cut / Copy / Paste =====
            if (ImGui::MenuItem(ICON_MI_FILE_IMPORT "  Import", "Ctrl+I")) {
                std::cout << "Import acionado!\n";
            }

            ImGui::Separator();

            // Cut / Copy / Paste
            if (ImGui::MenuItem(ICON_MI_SCISSORS "  Cut", "Ctrl+X")) {
                clipboardPath = currentDirectory; // aqui deveria ser item selecionado
                hasClipboard = true;
            }
            if (ImGui::MenuItem(ICON_MI_COPY "  Copy", "Ctrl+C")) {
                clipboardPath = currentDirectory; // item selecionado
                hasClipboard = true;
            }
            if (ImGui::MenuItem(ICON_MI_PASTE "  Paste", "Ctrl+V", hasClipboard)) {
                if (hasClipboard) {
                    try {
                        auto dest = currentDirectory / clipboardPath.filename();
                        std::filesystem::copy(clipboardPath, dest,
                            std::filesystem::copy_options::recursive);
                        std::cout << "Colado: " << dest << "\n";
                    }
                    catch (std::exception& e) {
                        std::cerr << "Erro ao colar: " << e.what() << "\n";
                    }
                }
            }

            ImGui::Separator();

            // Add New submenu
            if (ImGui::BeginMenu(ICON_MI_PLUS "  Add New")) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 130, 255, 255)); // roxo
                ImGui::TextUnformatted("Add New");
                ImGui::PopStyleColor();

                if (ImGui::MenuItem(ICON_MI_FOLDER "  Folder", "Ctrl+Shift+N")) {
                    std::filesystem::create_directory(currentDirectory / "New Folder");
                }
                if (ImGui::MenuItem(ICON_MI_FILE "  Scene")) {
                    std::ofstream(currentDirectory / "NewScene.scene");
                }
                if (ImGui::MenuItem(ICON_MI_BOX "  Prefab")) {
                    std::ofstream(currentDirectory / "NewPrefab.prefab");
                }
                if (ImGui::MenuItem(ICON_MI_FILL_DRIP "  Material")) {
                    std::ofstream(currentDirectory / "NewMaterial.mat");
                }
                if (ImGui::MenuItem(ICON_MI_IMAGE "  Texture")) {
                    std::ofstream(currentDirectory / "NewTexture.png");
                }
                if (ImGui::MenuItem(ICON_MI_MUSIC "  Audio")) {
                    std::ofstream(currentDirectory / "NewAudio.wav");
                }
                if (ImGui::MenuItem(ICON_MI_FILE_CODE "  C# Script")) {
                    std::ofstream(currentDirectory / "NewScript.cs");
                }
                if (ImGui::MenuItem(ICON_MI_FILM "  Animation")) {
                    std::ofstream(currentDirectory / "NewAnimation.anim");
                }

                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_MI_SYNC "  Refresh", "Ctrl+R")) {
                std::cout << "Refresh acionado!\n";
            }
            if (ImGui::MenuItem(ICON_MI_FOLDER_OPEN "  Open in Explorer")) {
                std::string cmd = "explorer " + currentDirectory.string();
                system(cmd.c_str());
            }

            ImGui::EndPopup();
        }
    }


    void Draw(SceneECS& scene) override {
        ImGui::Begin("Hierarchy");

        DrawHierarchyPopup("currentDirectory");

        for (Entity entity : scene.GetAllEntities()) {
            // Só renderiza se nao tiver pai (ou seja, é um nó raiz da hierarquia)
            if (scene.HasComponent<HierarchyComponent>(entity)) {
                const auto& hierarchy = scene.GetComponent<HierarchyComponent>(entity);
                if (hierarchy.parent == INVALID_ENTITY) {
                    DrawEntityNode(scene, entity);
                }
            }
            else {
                // Entidades sem HierarchyComponent sao tratadas como raiz também
                DrawEntityNode(scene, entity);
            }
        }
        ImGui::End();
    }

    const char* GetName() override { return "Scene"; }

private:

    void DrawEntityNode(SceneECS& scene, Entity entity) {
        const auto& name = scene.HasComponent<NameComponent>(entity) ? scene.GetComponent<NameComponent>(entity).name : "Unnamed Entity";

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (scene.GetSelectedEntity() == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool hasChildren = false;
        if (scene.HasComponent<HierarchyComponent>(entity)) {
            const auto& h = scene.GetComponent<HierarchyComponent>(entity);
            hasChildren = (h.firstChild != INVALID_ENTITY);
        }

        bool open = false;
        if (hasChildren)
            open = ImGui::TreeNodeEx((void*)(intptr_t)entity, flags, "%s", name.c_str());
        else
            ImGui::TreeNodeEx((void*)(intptr_t)entity, flags | ImGuiTreeNodeFlags_Leaf, "%s", name.c_str());

        if (ImGui::IsItemClicked())
            scene.SelectEntity(entity);

        // Desenha filhos recursivamente
        if (open && hasChildren) {
            Entity child = scene.GetComponent<HierarchyComponent>(entity).firstChild;
            while (child != INVALID_ENTITY) {
                DrawEntityNode(scene, child);
                child = scene.GetComponent<HierarchyComponent>(child).nextSibling;
            }
            ImGui::TreePop();
        }

        if (!hasChildren)
            ImGui::TreePop(); // necessário para manter o estado correto se não tiver filhos
    }


};




//void DrawHierarchyPopup(const std::string& currentDirectory) {
//    if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight)) {
//
//        // Import
//        if (ImGui::MenuItem(ICON_MI_FILE_IMPORT "  Import", "Ctrl+I")) {
//            // TODO: Abrir diálogo de import
//        }
//
//        ImGui::Separator();
//
//        // Cut / Copy / Paste
//        if (ImGui::MenuItem(ICON_MI_SCISSORS "  Cut", "Ctrl+X")) { /* TODO */ }
//        if (ImGui::MenuItem(ICON_MI_COPY "  Copy", "Ctrl+C")) { /* TODO */ }
//        if (ImGui::MenuItem(ICON_MI_PASTE "  Paste", "Ctrl+V")) { /* TODO */ }
//
//        ImGui::Separator();
//
//        // Add New submenu
//        if (ImGui::BeginMenu(ICON_MI_PLUS "  Add New")) {
//            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 130, 255, 255)); // roxo
//            ImGui::TextUnformatted("Add New");
//            ImGui::PopStyleColor();
//
//            if (ImGui::MenuItem(ICON_MI_FOLDER "  Folder", "Ctrl+Shift+N")) { /* TODO */ }
//            if (ImGui::MenuItem(ICON_MI_FILE "  Scene")) { /* TODO */ }
//            if (ImGui::MenuItem(ICON_MI_BOX "  Prefab")) { /* TODO */ }
//            if (ImGui::MenuItem(ICON_MI_FILL_DRIP "  Material")) { /* TODO */ }
//            if (ImGui::MenuItem(ICON_MI_IMAGE "  Texture")) { /* TODO */ }
//            if (ImGui::MenuItem(ICON_MI_MUSIC "  Audio")) { /* TODO */ }
//            if (ImGui::MenuItem(ICON_MI_FILE_CODE "  C# Script")) { /* TODO */ }
//            if (ImGui::MenuItem(ICON_MI_FILM "  Animation")) { /* TODO */ }
//
//            ImGui::EndMenu();
//        }
//
//        ImGui::Separator();
//
//        if (ImGui::MenuItem(ICON_MI_SYNC "  Refresh", "Ctrl+R")) { /* TODO */ }
//        if (ImGui::MenuItem(ICON_MI_FOLDER_OPEN "  Open in Explorer")) {
//            system(("explorer " + currentDirectory).c_str()); // Windows
//        }
//
//        ImGui::EndPopup();
//    }
//}
//
//
//// Simples handlers de copiar/colar
//inline static std::filesystem::path clipboardPath;
//inline static bool hasClipboard = false;
//
//void ShowContextMenu(const std::filesystem::path& currentDir) {
//    if (ImGui::BeginPopupContextWindow("FileExplorerContext", ImGuiPopupFlags_MouseButtonRight)) {
//        // ===== Categoria: Import / Copiar / Colar =====
//        if (MenuItemIcon("📂", "Import", "Ctrl+I")) {
//            std::cout << "Import acionado!\n";
//        }
//        if (MenuItemIcon("✂️", "Cut", "Ctrl+X")) {
//            clipboardPath = currentDir; // aqui deveria ser item selecionado
//            hasClipboard = true;
//        }
//        if (MenuItemIcon("📄", "Copy", "Ctrl+C")) {
//            clipboardPath = currentDir; // item selecionado
//            hasClipboard = true;
//        }
//        if (MenuItemIcon("📋", "Paste", "Ctrl+V", hasClipboard)) {
//            try {
//                if (hasClipboard) {
//                    auto dest = currentDir / clipboardPath.filename();
//                    std::filesystem::copy(clipboardPath, dest,
//                        std::filesystem::copy_options::recursive);
//                    std::cout << "Colado: " << dest << "\n";
//                }
//            }
//            catch (std::exception& e) {
//                std::cerr << "Erro ao colar: " << e.what() << "\n";
//            }
//        }
//
//        ImGui::Separator();
//
//        // ===== Categoria: Add New =====
//        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 130, 255, 255)); // Roxo
//        ImGui::Text("＋ Add New");
//        ImGui::PopStyleColor();
//
//        if (MenuItemIcon("📁", "Folder", "Ctrl+Shift+N")) {
//            auto newFolder = currentDir / "New Folder";
//            std::filesystem::create_directory(newFolder);
//        }
//        if (MenuItemIcon("🎬", "Scene")) {
//            std::ofstream(currentDir / "NewScene.scene");
//        }
//        if (MenuItemIcon("📦", "Prefab")) {
//            std::ofstream(currentDir / "NewPrefab.prefab");
//        }
//        if (MenuItemIcon("🎨", "Material")) {
//            std::ofstream(currentDir / "NewMaterial.mat");
//        }
//        if (MenuItemIcon("🖼️", "Texture")) {
//            std::ofstream(currentDir / "NewTexture.png");
//        }
//        if (MenuItemIcon("🎼", "Audio")) {
//            std::ofstream(currentDir / "NewAudio.wav");
//        }
//        if (MenuItemIcon("📜", "C# Script")) {
//            std::ofstream(currentDir / "NewScript.cs");
//        }
//        if (MenuItemIcon("🎞️", "Animation")) {
//            std::ofstream(currentDir / "NewAnimation.anim");
//        }
//
//        ImGui::Separator();
//
//        // ===== Categoria: Utilidades =====
//        if (MenuItemIcon("🔄", "Refresh", "Ctrl+R")) {
//            std::cout << "Refresh acionado!\n";
//        }
//        if (MenuItemIcon("📂", "Open in Explorer")) {
//            std::string cmd = "explorer " + currentDir.string();
//            system(cmd.c_str());
//        }
//
//        ImGui::EndPopup();
//    }
//}


 





//void popup() {
//    // Clique com botão direito em qualquer área vazia da janela
//    if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight)) {
//        if (ImGui::MenuItem("Import", "Ctrl+I")) {
//            // TODO: Abrir diálogo de import
//        }
//        if (ImGui::MenuItem("Cut", "Ctrl+X")) {
//            // TODO: Cortar entidade selecionada
//        }
//        if (ImGui::MenuItem("Copy", "Ctrl+C")) {
//            // TODO: Copiar entidade selecionada
//        }
//        if (ImGui::MenuItem("Paste", "Ctrl+V")) {
//            // TODO: Colar entidade
//        }
//
//        if (ImGui::BeginMenu("Add New")) {
//            if (ImGui::MenuItem("Folder", "Ctrl+Shift+N")) {
//                // TODO: Criar nova pasta
//            }
//            if (ImGui::MenuItem("Scene")) {
//                // TODO: Criar nova cena
//            }
//            if (ImGui::MenuItem("Prefab")) {
//                // TODO: Criar prefab
//            }
//            if (ImGui::MenuItem("Material")) {
//                // TODO: Criar material
//            }
//            if (ImGui::MenuItem("Texture")) {
//                // TODO: Criar textura
//            }
//            if (ImGui::MenuItem("C# Script")) {
//                // TODO: Criar script
//            }
//            if (ImGui::MenuItem("Animation")) {
//                // TODO: Criar animação
//            }
//            ImGui::EndMenu();
//        }
//
//        ImGui::Separator();
//        if (ImGui::MenuItem("Refresh", "Ctrl+R")) {
//            // TODO: Recarregar hierarquia
//        }
//        if (ImGui::MenuItem("Open in Explorer")) {
//            //system(("explorer " + currentDirectory).c_str()); // Windows
//        }
//
//        ImGui::EndPopup();
//    }
//}



//for (auto entity : scene.GetAllEntities()) {
    //if (ImGui::Selectable(entity.GetName().c_str(), entity == scene.GetSelectedEntity())) {
    /*if (ImGui::Selectable("entity", entity == scene.GetSelectedEntity())) {
        scene.SelectEntity(entity);
    }*/
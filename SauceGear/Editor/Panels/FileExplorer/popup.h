#include <fstream>
#include <filesystem>
#include "../../Icons/MaterialIcons.h"  
#include "../IPanel.h"


static void DrawFilePopup(const std::filesystem::path& currentDirectory) {
    static std::filesystem::path clipboardPath;
    static bool hasClipboard = false;

    if (ImGui::BeginPopupContextWindow("File Explorer", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {

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

// ==== POPUP DO ITEM ====
static void DrawFileItemPopup(const std::filesystem::path& itemPath) {
    //if (ImGui::BeginPopup("FileItemPopup")) {
    if (ImGui::BeginPopupContextItem("File Explorer", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Open")) {
            std::cout << "Abrir: " << itemPath << "\n";
        }
        if (ImGui::MenuItem("Rename")) {
            std::cout << "Renomear: " << itemPath << "\n";
        }
        if (ImGui::MenuItem("Delete")) {
            try {
                std::filesystem::remove_all(itemPath);
                std::cout << "Deletado: " << itemPath << "\n";
            }
            catch (std::exception& e) {
                std::cerr << "Erro: " << e.what() << "\n";
            }
        }
        ImGui::EndPopup();
    }
}

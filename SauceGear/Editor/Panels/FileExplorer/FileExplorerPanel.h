#pragma once
#include <filesystem>
#include "../Load.h" 
#include "popup.h"
#include "../../Engine/ECS/Components/ComponentsHelper.h" 
#include "../../Engine/Graphics/Renderer.h"   

struct FileExplorerPanel : IPanel {
    enum class ViewMode {
        Hierarchy, // Modo hierárquico (nomes apenas)
        Icons,      // Modo com ícones e miniaturas
        HierarchyIcons
    };

    FileExplorerPanel() {
        stbi_set_flip_vertically_on_load(false);
        FolderIcon = guiLoadTexture("assets/icons/FolderIcon.png");
        FileIcon   = guiLoadTexture("assets/icons/FileIcon.png");
        ImageIcon  = guiLoadTexture("assets/icons/ImageIcon.png");
        CodeIcon   = guiLoadTexture("assets/icons/CodeIcon.png");

    }

    ViewMode currentMode = ViewMode::HierarchyIcons;

    void Draw(SceneECS& scene) override {
        ImGui::Begin("File Explorer"); 

        if (ImGui::Button(ICON_MD_HOME)) {
            currentMode = ViewMode::HierarchyIcons;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_MD_INFO)) {
            currentMode = ViewMode::Icons;
        }
        ImGui::SameLine();
        // Seleção do modo de visualização
        if (ImGui::Button(ICON_MD_BUILD)) {
            currentMode = ViewMode::Hierarchy;
        }

        // Dependendo do modo de visualização, chamamos as funções de desenho apropriadas
        if (currentMode == ViewMode::Hierarchy) {
            DrawHierarchyMode();
        }else if (currentMode == ViewMode::HierarchyIcons) {
            DrawIconHierarchyMode();
        }else {
            DrawIconsMode();
        }

        ImGui::End();
    }

    const char* GetName() override { return "File Explorer"; }

private:
    ImTextureID FolderIcon = NULL;
    ImTextureID FileIcon = NULL;
    ImTextureID ImageIcon = NULL;
    ImTextureID CodeIcon = NULL;

    void DrawHierarchyMode() {
        // Exibe os arquivos e pastas no modo hierárquico (compactado)
        for (auto& entry : std::filesystem::directory_iterator("Assets")) {
            if (entry.is_directory()) {
                // Se for um diretório, criamos um item de árvore
                bool isSelected = false;  // Flag para saber se a pasta foi selecionada
                if (ImGui::TreeNodeEx(entry.path().filename().string().c_str())) {      //, ImGuiTreeNodeFlags_OpenOnDoubleClick
                    // Verifica se o item foi clicado
                    if (ImGui::IsItemClicked()) {
                        // Atualiza o diretório atual
                        //currentDirectory = entry.path().string();
                        isSelected = true;
                    }

                    // Altera o estilo do item se ele for selecionado
                    if (isSelected) {
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.5f, 1.0f, 1.0f));  // Alterando a cor do item
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
                    }

                    // Chama recursivamente para exibir os arquivos dentro do diretório
                    for (auto& subEntry : std::filesystem::directory_iterator(entry.path())) {
                        // Exibe o nome do arquivo ou subdiretório
                        if (subEntry.is_directory()) {
                            ImGui::Text("%s", subEntry.path().filename().string().c_str());
                        }
                        else {
                            // Exibe arquivos como itens clicáveis
                            if (ImGui::Selectable(subEntry.path().filename().string().c_str())) {
                                // Atualiza o diretório quando um arquivo é clicado (como no Unity)
                                currentDirectory = entry.path().string();
                            }
                        }
                    }

                    // Finaliza a alteração do estilo, se necessário
                    if (isSelected) {
                        ImGui::PopStyleColor(2);  // Remove a modificação do estilo
                    }

                    ImGui::TreePop();
                }
            }
            else {
                // Se for um arquivo, exibe seu nome
                //ImGui::Text("%s", entry.path().filename().string().c_str());

                // Se for um arquivo, exibe seu nome com destaque ao passar o mouse
                if (ImGui::Selectable(entry.path().filename().string().c_str())) {
                    // Adicione o que deve ocorrer ao clicar em um arquivo, se necessário
                    // 
                    // Atualiza o diretório ao clicar em um arquivo
                    currentDirectory = entry.path().string();
                }
            }
        }
    }


    std::string currentDirectory = "Assets";  // Caminho atual da pasta
    void DrawIconHierarchyMode() {
        // Início da janela
        ImGui::Begin("File Explorer");

        // Obtém o espaço disponível para a largura da janela
        float availableWidth = ImGui::GetContentRegionAvail().x;

        // Define a largura inicial do painel de hierarquia (fixa ou com valor default)
        static float leftPanelWidth = 200.0f;
        const float minPanelWidth = 100.0f;  // Largura mínima para o painel esquerdo
        const float maxPanelWidth = availableWidth - 50.0f;  // Largura máxima para o painel esquerdo

        // Começa o painel esquerdo para a hierarquia com a largura configurada
        ImGui::BeginChild("LeftPanel", ImVec2(leftPanelWidth, 0), true);  // O painel esquerdo é redimensionável
        DrawHierarchyMode();  // Desenha a hierarquia
        ImGui::EndChild();

        // Espaço para mover o painel de hierarquia
        ImGui::SameLine();

        // Começa o painel direito para os ícones com o restante da largura disponível
        float rightPanelWidth = availableWidth - leftPanelWidth;
        ImGui::BeginChild("RightPanel", ImVec2(rightPanelWidth, 0), false);  // O painel direito ocupa o restante do espaço 

        DrawFilePopup(currentDirectory);

        DrawIconsMode();  // Desenha os ícones de arquivos
        ImGui::EndChild();

        // Permitir redimensionar a largura do painel esquerdo (hierarquia) com o mouse
        if (ImGui::IsItemHovered() && ImGui::IsMouseDown(0)) {
            float delta = ImGui::GetIO().MouseDelta.x;  // Calcula o movimento do mouse
            leftPanelWidth = std::clamp(leftPanelWidth + delta, minPanelWidth, maxPanelWidth);  // Restringe o valor do redimensionamento
        }

        ImGui::End();
    }

    void DrawIconsMode() {
        // Início da janela
        //ImGui::Begin("File Explorer");

        // Configurar as colunas (número de ícones por linha)
        //const int columns = 5;  // Número de colunas
        float iconSize = 48.0f; // Tamanho do ícone
        float padding = 30.0f;  // Espaço entre os ícones

        // Obter a largura disponível da janela e calcular o número de colunas
        float availableWidth = ImGui::GetContentRegionAvail().x;
        int columns = (int)std::floor(availableWidth / (iconSize + padding));  // Calcula quantas colunas cabem
        //int columns = (int)(availableWidth / (iconSize + padding));  // Calcula quantas colunas cabem

        // Garante que ao menos 1 coluna seja exibida
        columns = std::max(columns, 1);

        // Definindo o layout da coluna
        ImGui::Columns(columns, nullptr, false);  // Desabilita a borda, 5 colunas

        static std::filesystem::path selectedItem;
        for (auto& entry : std::filesystem::directory_iterator(currentDirectory)) {     //"Assets"
            ImGui::PushID(entry.path().filename().string().c_str());

            // Definir o ícone dependendo do tipo de arquivo
            bool isFolder = entry.is_directory();

            if (isFolder) {
                if (ImGui::ImageButton("FolderIcon", FolderIcon, ImVec2(iconSize, iconSize))) {
                    //if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                        std::cout << "Abrir pasta: " << entry.path().string() << std::endl;
                        // Se for uma pasta, muda para ela
                        currentDirectory = entry.path().string();
                    //}
                }
            }
            else {
                std::string fileExt = entry.path().extension().string();
                if (fileExt == ".png" || fileExt == ".jpg" || fileExt == ".jpeg") {
                    // Imagem
                    if (ImGui::ImageButton("ImageIcon", ImageIcon, ImVec2(iconSize, iconSize))) {
                        std::cout << "Abrir imagem: " << entry.path().string() << std::endl;
                    }
                }
                else if (fileExt == ".cpp" || fileExt == ".h") {
                    // Código fonte (ícone de script)
                    if (ImGui::ImageButton("CodeIcon", CodeIcon, ImVec2(iconSize, iconSize))) {
                        std::cout << "Abrir código: " << entry.path().string() << std::endl;
                    }
                }
                else {
                    // Ícone genérico para arquivos desconhecidos
                    if (ImGui::ImageButton("FileIcon", FileIcon, ImVec2(iconSize, iconSize))) {
                        std::cout << "Abrir arquivo: " << entry.path().string() << std::endl;
                    }
                }
            }

            // Guardar path pra usar no popup
            selectedItem = entry.path();
            DrawFileItemPopup(selectedItem);

            // Exibe o nome do arquivo abaixo do ícone
            ImGui::Text("%s", entry.path().filename().string().c_str());

            // Avança para a próxima coluna na grade
            ImGui::NextColumn();

            ImGui::PopID();
        }

        // Finaliza as colunas (retorna ao layout normal)
        ImGui::Columns(1);

        //ImGui::End();
    }



};




// Indentação para representar a profundidade da pasta
//ImGui::Indent(level * 20);
//ImGui::Unindent(level * 20);


















//struct FileExplorerPanel : IPanel {
//
//    void Draw(SceneECS& scene) override {
//        ImGui::Begin("Assets");
//        ImGui::Text("Navegador de arquivos...");
//
//        for (auto& entry : std::filesystem::directory_iterator("Assets")) {
//            ImGui::Text("%s", entry.path().filename().string().c_str());
//
//            //ImGui::Selectable(asset.name.c_str());
//            //if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
//            //    // abrir asset ou instanciar na cena
//            //}
//        }
//
//        ImGui::End();
//    }
//
//    const char* GetName() override { return "Scene"; }
//
//};








//ImGui::Begin("Assets");
//ImGui::Text("Navegador de arquivos...");
//
//std::vector<std::string> files;
//std::vector<std::string> dirs;
//
//// Filtra os arquivos e diretórios
//for (const auto& entry : fs::directory_iterator("Assets")) {
//    if (fs::is_directory(entry)) {
//        dirs.push_back(entry.path().filename().string());
//    }
//    else {
//        files.push_back(entry.path().filename().string());
//    }
//}
//
//// Exibindo diretórios primeiro
//for (const auto& dir : dirs) {
//    if (ImGui::Button(dir.c_str())) {
//        // Aqui você pode implementar a navegação nas pastas
//        std::cout << "Diretório: " << dir << std::endl;
//    }
//    ImGui::SameLine();
//}
//
//// Agora exibindo os arquivos
//for (const auto& file : files) {
//    if (ImGui::Button(file.c_str())) {
//        // Aqui você pode implementar ações quando um arquivo for selecionado
//        std::cout << "Arquivo: " << file << std::endl;
//    }
//    ImGui::SameLine();
//}
//
//ImGui::End();
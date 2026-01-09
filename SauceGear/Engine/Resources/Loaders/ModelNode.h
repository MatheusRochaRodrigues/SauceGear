    #pragma once
    #include <glm/glm.hpp>
    #include <string>
    #include <iostream>
    #include "../../Assets/MeshAsset.h"

    //Descritor de arquivo
    struct ModelNode {      //hierarquia local DO ARQUIVO
        std::string name;
        glm::mat4 localTransform{1.0f};

        // índices para meshes do ModelAsset
        std::vector<uint32_t> meshIndices;

        std::vector<std::shared_ptr<ModelNode>> children;


        void CopyFrom(const ModelNode& src) {
            localTransform = src.localTransform;
            name = src.name;

            children.resize(src.children.size());
            for (size_t i = 0; i < children.size(); ++i) {
                if (!children[i])
                    children[i] = std::make_shared<ModelNode>();

                children[i]->CopyFrom(*src.children[i]);
            }
        }
    };

#pragma once
#include "SceneECS.h"
#include "../Core/EngineContext.h"
#include "../ECS/Components/ComponentsHelper.h"
#include "../Resources/Model.h"  
#include "../Utils/AABBBuilder.h"

class SceneBuilder {       //GameObjectFactory
public: 
    //Special Create tools GameOBJ 
    static Entity CreateGameObject(string name = "GameObject") {
        auto& scene = *GEngine->scene;
        Entity entity = scene.CreateEntity();
        //Special Components
        scene.AddComponent<NameComponent>(entity).name = name;                      //AddComponent<NameComponent>(entity, name); 
        scene.AddComponent<Transform>(entity);

        return entity;
    }

    static Entity CreateModel(string url, std::shared_ptr<MaterialInstance> materialOverride = nullptr) {
        Mesh* mesh = ModelLoader::LoadModel(url);
        if (mesh == nullptr) {
            std::cout << "Mesh vazia para a entidade" << std::endl;
            return INVALID_ENTITY;
        }
        Entity i = InstantiateNode(mesh, INVALID_ENTITY, materialOverride);

        GEngine->scene->AddComponent<AABBComponent>(i); 
        //AABBBuilder::aabbMesh(i);

        return i;
    }

    static Entity CreateModel(Mesh* mesh, std::shared_ptr<MaterialInstance> materialOverride = nullptr) {
        if (mesh == nullptr) {
            std::cout << "Mesh vazia para a entidade" << std::endl;
            return INVALID_ENTITY;
        }
        Entity i = InstantiateNode(mesh, INVALID_ENTITY, materialOverride);

        GEngine->scene->AddComponent<AABBComponent>(i);
        //AABBBuilder::aabbMesh(i);

        return i;
    }



private:

private:
    static Entity InstantiateNode(Mesh* meshNode, Entity father = INVALID_ENTITY, std::shared_ptr<MaterialInstance> materialOverride = nullptr) {
        if (!meshNode) return INVALID_ENTITY;

        auto& scene = *GEngine->scene;
        Entity e = scene.CreateEntity();

        scene.AddComponent<NameComponent>(e).name = (!meshNode->name.empty()) ? meshNode->name : "GameObjectModel";
        auto& tr = scene.AddComponent<Transform>(e);

        //          Versao que adiciona mesh renderere apenas se a mesh possuir vertice
        if (!meshNode->submeshes.empty() && !meshNode->vertices.empty()) {
            auto& mr = scene.AddComponent<MeshRenderer>(e, meshNode);
            // ⚡ aplica material customizado (se existir)
            if (materialOverride) {
                for (auto& sm : meshNode->submeshes) {
                    sm.material = materialOverride;
                }
            }
            mr.RebuildBatches();
        }
        else {
            std::cout << " mesh vazia logo sem mesh renderer " << std::endl;
            auto& mr = scene.AddComponent<MeshRenderer>(e); 
        }
        

        //          Versao que adiciona mesh rendererer em todos os casos
        /*
        auto& mr = scene.AddComponent<MeshRenderer>(e);
        if (meshNode && !meshNode->submeshes.empty()) {
            mr.SetMesh(meshNode);
            if (materialOverride) {
                for (auto& sm : meshNode->submeshes) sm.material = materialOverride;
            }
        }
        */



        // Recursão para filhos
        for (auto* child : meshNode->children)
            InstantiateNode(child, e, materialOverride);

        if (father != INVALID_ENTITY)
            scene.AddToParent(father, e);

        return e;
    }




    // Instancia a árvore de Mesh no ECS
    // - 1 entidade por node (igual Unity).
    // - Se o node tiver geometria (submeshes > 0), recebe MeshFilter+MeshRenderer.
    static Entity InstantiateMeshTree(Mesh* root, Entity parent = INVALID_ENTITY, bool splitSubmeshesIntoChildren = false) {
        if (!root) return INVALID_ENTITY;

        auto& scene = *GEngine->scene;

        Entity e = scene.CreateEntity();

        scene.AddComponent<NameComponent>(e).name = (!root->name.empty()) ? root->name : "GameObjectModel";
        auto& tr = scene.AddComponent<Transform>(e);

        // Transform identidade por padrão (se quiser TRS do FBX, guardar no Mesh e setar aqui)
        /*tr.position = meshNode->localPosition;
        tr.rotationQuat = meshNode->localRotation;
        tr.scale = meshNode->localScale;*/

        // Se este node tem geometria:
        if (!root->submeshes.empty() && !root->vertices.empty()) {
            if (!splitSubmeshesIntoChildren) {
                // 1 entidade contendo toda a geometria do node
                //auto& mf = scene.AddComponent<MeshFilter>(e, root); // usa construtor MeshFilter(Mesh*)
                auto& mr = scene.AddComponent<MeshRenderer>(e, root);     // construtor que pega filter via entity
                //mr.SetFilter(&mf);
                mr.RebuildBatches(); // agrupa por material


            } else {
                // opcional: 1 entidade-filho por submesh (normalmente NÃO é necessário)
                for (size_t i = 0; i < root->submeshes.size(); ++i) {
                    Entity child = CreateGameObject("SubMesh " + std::to_string(i));
                    scene.AddToParent(e, child);

                    // cria uma view (mesh “leve”) apontando pro mesmo VAO/EBO, mas com um único SubMesh
                    auto* view = new Mesh();
                    view->name = root->name + "_sm" + std::to_string(i);
                    view->directory = root->directory;
                    view->VAO = root->VAO; // compartilha buffers (atenção ao lifetime!)
                    view->vertices = root->vertices; // compartilha (ou deixe vazio; só precisa do draw range)
                    view->indices = root->indices;
                    view->submeshes.push_back(root->submeshes[i]);

                    //auto& mf = scene.AddComponent<MeshFilter>(child, view);
                    auto& mr = scene.AddComponent<MeshRenderer>(child, view);
                    //mr.SetFilter(&mf);
                    mr.RebuildBatches();
                }
            }

        }

        // Recursão pros filhos
        for (auto* c : root->children) {
            if (!c) continue;
            Entity ce = InstantiateMeshTree(c, e, splitSubmeshesIntoChildren);
            scene.AddToParent(e, ce);
        }

        //if (parent != INVALID_ENTITY) scene.AddToParent(parent, e);
        return e;
    }

     
};







#pragma once
#include "SceneECS.h"
#include "../Core/EngineContext.h"
#include "../Scene/Systems/SystemHelper.h"
#include "../Resources/Model.h"  

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

    static Entity CreateModel(string url) { 
        Mesh* mesh = ModelLoader::LoadModel(url);
        if (mesh == nullptr) { 
            std::cout << "mesh Vazia para a entidade" << std::endl;
            return INVALID_ENTITY; }
        std::cout << "Testando aq  21312 " << mesh->name << std::endl;
        return InstantiateNode(mesh);
    }

    static Entity CreateModel(Mesh* mesh) { 
        if (mesh == nullptr) {
            std::cout << "mesh Vazia para a entidade" << std::endl;
            return INVALID_ENTITY;
        } 
        std::cout << "Testando aq " << mesh->name << std::endl;
        return InstantiateNode(mesh);
    }


private:

    static Entity InstantiateNode(Mesh* meshNode, Entity father = INVALID_ENTITY) {
        std::cout << "mesh  d " << meshNode->name << std::endl;
        if (!meshNode) return INVALID_ENTITY;

        auto& scene = *GEngine->scene;
        Entity e = scene.CreateEntity(); 

        scene.AddComponent<NameComponent>(e).name = (!meshNode->name.empty()) ? meshNode->name : "GameObjectModel";  
        // Aplica TRS carregado do FBX
        auto& tr = scene.AddComponent<Transform>(e); 

        // Se essa mesh tem geometria, cria MeshFilter/MeshRenderer
        if (!meshNode->submeshes.empty() && !meshNode->vertices.empty()) {
            //auto& mf = scene.AddComponent<MeshFilter>(e);
            auto& mr = scene.AddComponent<MeshRenderer>(e, meshNode);
            //mf.meshes.push_back(meshNode);
            //mr.SyncWithMesh(mf);
            mr.RebuildBatches(); // agrupa por material
        }

        // Recursão para filhos
        for (auto* child : meshNode->children)
            InstantiateNode(child, e);

        // Vincula ao pai
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







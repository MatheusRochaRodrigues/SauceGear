#pragma once
#include "SceneECS.h"
#include "../Core/EngineContext.h"
#include "../ECS/Components/ComponentsHelper.h"
#include "../Resources/Loaders/ModelLoader.h"  
#include "../Utils/AABBBuilder.h"
#include "../Assets/AssetLoader.h"
#include "../Instancing/ModelInstance.h"
#include "../ECS/Components/MeshComponent.h"

class SceneBuilder {       //GameObjectFactory
public: 
    //Special Create tools GameOBJ 
    static Entity CreateGameObject(string name = "GameObject") {
        auto& scene = *GEngine->scene;
        Entity entity = scene.CreateEntity();
        //Special Components
        scene.AddComponent<NameComponent>(entity).name = name;                      //AddComponent<NameComponent>(entity, name); 
        scene.AddComponent<TransformComponent>(entity);

        return entity;
    }
     
    //Special Create tools GameOBJ 
    static Entity CreateGUIText(string name = "Text") {
        auto& scene = *GEngine->scene;
        Entity entity = scene.CreateEntity();
        //Special Components
        scene.AddComponent<NameComponent>(entity).name = name;                      //AddComponent<NameComponent>(entity, name); 
        scene.AddComponent<TransformComponent>(entity);
        scene.AddComponent<TextComponent>(entity);

        return entity;
    }

    // ===============================
    // Model from file
    // ===============================
    /*static Entity CreateModelSingle(const std::string& path) {
        auto model = LoadAsset<ModelAsset>(path);
        ASSERT(!model->meshes.empty());

        auto instance = std::make_shared<MeshInstance>(model->meshes[0]);

        auto& scene = *GEngine->scene;
        Entity root = scene.CreateEntity();

        scene.AddComponent<NameComponent>(root).name = model->name;
        scene.AddComponent<TransformComponent>(root);
        scene.AddComponent<ModelComponent>(root).instance = instance;

        return root;
    }*/

     
    static Entity CreateModel(const std::string& path) {
        std::cout << "pre m" << std::endl;
        auto model = LoadAsset<ModelAsset>(path);
         
        if (!model || !model->root) {
            LOG_ERROR("Falha ao carregar model {}", path);
            return INVALID_ENTITY;
        } 

        std::cout << "pos m" << std::endl;
        return InstantiateNode(model->root, model, INVALID_ENTITY);
    }
     

     
    static Entity InstantiateNode(
        const std::shared_ptr<ModelNode>& node,
        const std::shared_ptr<ModelAsset>& model,
        Entity parent
    ) {
        auto& scene = *GEngine->scene;

        Entity e = scene.CreateEntity();

        scene.AddComponent<NameComponent>(e).name = node->name;

        auto& tr = scene.AddComponent<TransformComponent>(e);
        tr.SetLocalFromMatrix(node->localTransform);            //tr.local = node->localTransform;

        auto& mr = scene.AddComponent<MeshRenderer>(e);

        // um node pode ter N meshes
        for (uint32_t meshIndex : node->meshIndices) { 
            auto meshAsset = model->meshes[meshIndex]; 
            mr.mesh = std::make_shared<MeshInstance>(meshAsset);        //mr.mesh = MeshInstanceCache::Get(meshAsset);

            for (auto& sm : meshAsset->submeshes)
                if (sm.materialAsset) {
                    auto inst = sm.materialAsset->Instantiate();
                    if (inst) mr.materials.push_back(inst);
                } 
        }

        if (parent != INVALID_ENTITY) scene.AddToParent(parent, e);

        for (auto& c : node->children) InstantiateNode(c, model, e);

        return e;
    }


    Entity CreateCube();

};












// ============================================================
    // Cria uma entidade ECS a partir de um Mesh (recursivo)
    // ============================================================
    //static Entity InstantiateMeshNode( Mesh* meshNode, Entity parent, std::shared_ptr<MaterialInstance> materialOverride ) {
    //    if (!meshNode) return INVALID_ENTITY;

    //    auto& scene = *GEngine->scene;
    //    Entity e = scene.CreateEntity();

    //    // Nome + Transform
    //    scene.AddComponent<NameComponent>(e).name =
    //        meshNode->name.empty() ? "ModelNode" : meshNode->name;

    //    scene.AddComponent<TransformComponent>(e);

    //    // ===============================
    //    // MeshRenderer (somente se houver geometria)
    //    // ===============================
    //    if (!meshNode->submeshes.empty() && !meshNode->vertices.empty()) {

    //        auto& mr = scene.AddComponent<MeshRenderer>(e);
    //        mr.mesh = meshNode;

    //        // Cria MaterialInstance a partir dos MaterialAssets
    //        mr.BuildFromMesh();

    //        // Override opcional (runtime-only)
    //        if (materialOverride) {
    //            for (auto& inst : mr.materials)
    //                inst = materialOverride;
    //        }
    //    }

    //    // ===============================
    //    // Hierarquia
    //    // ===============================
    //    if (parent != INVALID_ENTITY)
    //        scene.AddToParent(parent, e);

    //    for (auto* child : meshNode->children) {
    //        InstantiateMeshNode(child, e, materialOverride);
    //    }

    //    return e;
    //} 
#pragma once
#include "SceneECS.h"
#include "../Core/EngineContext.h"

#include "../ECS/Components/AABBComponent.h"
#include "../ECS/Components/HierarchyComponent.h"
#include "../ECS/Components/OutlineComponent.h"
#include "../ECS/Components/TextComponent.h"
#include "../ECS/Components/MeshComponent.h"

#include "../Resources/Loaders/ModelLoader.h"  
#include "../Utils/AABBBuilder.h"
#include "../Assets/AssetLoader.h"
#include "../Instancing/ModelInstance.h"
#include "../Materials/MaterialLibrary.h"

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

    static Entity CreateModel(const std::shared_ptr<MeshAsset> mesh, std::shared_ptr<MaterialInstance> mat = nullptr) {
        if (!mesh) {
            std::cout << "Mesh vazia para a entidade" << std::endl;
            return INVALID_ENTITY;
        }   
        auto& scene = *GEngine->scene;
        Entity entity = scene.CreateEntity();
        //Special Components

        scene.AddComponent<NameComponent>(entity).name = mesh->name;
        scene.AddComponent<TransformComponent>(entity);

        auto& mr = scene.AddComponent<MeshRenderer>(entity);
        mr.mesh = std::make_shared<MeshInstance>(mesh);

        if (!mat) {
            auto asset = std::make_shared<MaterialAsset>();
            asset->name = mesh->name;  // name 
            asset->base = MaterialLibrary::Get("PBR_Default");
            float value = 1;
            asset->defaults["Albedo"].data = TextureCache::Get().GetSolidColor(glm::vec4(value, value, value, 1));
            value = 0.2f;
            asset->defaults["Metallic"].data = TextureCache::Get().GetSolidColor(glm::vec4(value, value, value, 1));
            value = 0.5f;
            asset->defaults["Roughness"].data = TextureCache::Get().GetSolidColor(glm::vec4(value, value, value, 1));

            mat = MaterialAsset::Instantiate(asset);
            //if (!mat) 
        } 
        mr.materials.push_back(mat);  
        mr.BuildBatches(); // custo pago UMA vez

        scene.AddComponent<AABBComponent>(entity);
        scene.AddComponent<OutlineComponent>(entity);

        return entity;
    }

     
    static Entity CreateModel(const std::string& path) {
        std::cout << "pre m" << std::endl;
        auto model = LoadAsset<ModelAsset>(path);
         
        if (!model || !model->root) {
            LOG_ERROR("Falha ao carregar model {}", path);
            return INVALID_ENTITY;
        } 

        std::cout << "pos m" << std::endl;
        return InstantiateNode(model, model->root, INVALID_ENTITY);
    } 

     
    static Entity InstantiateNode(
        const std::shared_ptr<ModelAsset>& model,
        const std::shared_ptr<HierarchyNode>& node,
        Entity parent
    ) {
        auto& scene = *GEngine->scene;
        Entity e = scene.CreateEntity();

        scene.AddComponent<NameComponent>(e).name = node->name;

        auto& tr = scene.AddComponent<TransformComponent>(e);
        tr.SetLocalFromMatrix(node->localTransform);

        if (!node->meshIndices.empty()) {
            for (uint32_t meshIndex : node->meshIndices) {
                auto& mr = scene.AddComponent<MeshRenderer>(e);

                auto meshAsset = model->meshes[meshIndex];
                mr.mesh = std::make_shared<MeshInstance>(meshAsset);

                mr.materials.resize(model->materials.size());

                for (const auto& sm : meshAsset->submeshes) {
                    uint32_t matIndex = sm.indexMaterialAsset;

                    if (matIndex < model->materials.size())
                        mr.materials[matIndex] = MaterialAsset::Instantiate(model->materials[matIndex]);
                }

                mr.BuildBatches(); // custo pago UMA vez
            }
        }

        if (parent != INVALID_ENTITY) scene.AddToParent(parent, e);

        for (auto& child : node->children)
            InstantiateNode(model, child, e);

        return e;
    }
     
    Entity CreateCube();
};




/*

static Entity InstantiateNode(
        const std::shared_ptr<ModelAsset>& model,
        const std::shared_ptr<HierarchyNode>& HNode,
        Entity parent
    ) {
        auto& scene = *GEngine->scene;
        Entity e = scene.CreateEntity();

        scene.AddComponent<NameComponent>(e).name = HNode->name;

        auto& tr = scene.AddComponent<TransformComponent>(e);
        tr.SetLocalFromMatrix(HNode->localTransform);

        // Apenas CRIA MeshRenderer se o node tiver meshes
        if (!HNode->meshIndices.empty()) {
            auto& mr = scene.AddComponent<MeshRenderer>(e);
            //each index mesh == one mesh
            for (uint32_t meshIndex : HNode->meshIndices) {
                auto meshAsset = model->meshes[meshIndex];  // real mesh
                if (!meshAsset) continue;

                mr.mesh = std::make_shared<MeshInstance>(meshAsset);
                //mr.materials.reserve(meshAsset->submeshes.size());

                // 1 material por submesh (ordem IMPORTA)
                for (const SubMesh& sm : meshAsset->submeshes) {
                    uint32_t matIndex = sm.indexMaterialAsset;

                    if (matIndex < model->materials.size())
                        mr.materials[matIndex] = (matIndex, MaterialAsset::Instantiate(model->materials[matIndex]) );
                    else
                        mr.materials[matIndex] = (nullptr); // fallback seguro
                }

            }
        }

        if (parent != INVALID_ENTITY) scene.AddToParent(parent, e);
        for (auto& cnode : HNode->children) InstantiateNode(model, cnode, e);

        return e;
    }

*/







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
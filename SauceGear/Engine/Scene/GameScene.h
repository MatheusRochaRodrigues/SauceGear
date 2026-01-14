#pragma once
#include "SceneECS.h" 
#include "../Graphics/Framebuffer.h" 
#include "../ECS/Components/ComponentsHelper.h"
#include <stdexcept> 
#include "SceneBuilder.h"
#include "../Geometry/World/SurfaceNets/Voxel.h"
#include <memory>

#include"../ECS/Systems/DebugRenderer.h"
#include "../GUI/FontManager.h" 
  
class GameScene : public SceneECS {
public: 
    void Load() override {
        // --- Main Camera ---
        Entity cameraEntity = CreateEntity();
        auto& cameraTransform = AddComponent<TransformComponent>(cameraEntity);
        auto& camComp = AddComponent<CameraComponent>(cameraEntity);
        AddComponent<NameComponent>(cameraEntity).name = "Camera";
        camComp.isMain = true;

        {
            computeManager = CreateEntity();
            AddComponent<ComputeSyncComponent>(computeManager);
            std::cout << "[INFO Scene] ComputeManager entity criado: " << computeManager << "\n";
        }

        {
            stbi_set_flip_vertically_on_load(true);
            std::cout << "cs 1" << std::endl;

            Entity entity = SceneBuilder::CreateModel("Engine/Resources/Models/backpack/backpack.obj");
            //Entity entity = SceneBuilder::CreateModel("Engine/Resources/Models/Skull/Skull.obj");
            std::cout << "cs 2" << std::endl;

            auto& l = GetComponent<TransformComponent>(entity);
            l.position = glm::vec3(1, 1, 0);  
        }

        {
            Entity pointLight = SceneBuilder::CreateGameObject("Sun");
            auto& pTransform = GetComponent<TransformComponent>(pointLight); 
            pTransform.rotation = glm::vec3(21.0f, 0.0f, 1.0f); 
            pTransform.scale = glm::vec3(0.05, 0.05, 0.05);
            pTransform.position = glm::vec3(20.0f, 50, 20.0f);
            auto& pLight = AddComponent<LightComponent>(pointLight); 
            pLight.SetTypeLight(LightType::Directional);
            pLight.color = glm::vec3(1, 0, 1);
            pLight.intensity = 1.0f;
        }
        {
            Entity pointLight = SceneBuilder::CreateGameObject("Light2");
            auto& pTransform = GetComponent<TransformComponent>(pointLight);
            pTransform.rotation = glm::vec3(-2.0f, 4.0f, -1.0f); 
            pTransform.scale = glm::vec3(0.05, 0.05, 0.05);
            pTransform.position = glm::vec3(0, 0, 0);
            auto& pLight = AddComponent<LightComponent>(pointLight); 
            pLight.SetTypeLight(LightType::Point);
            pLight.color = glm::vec3(0, 1, 0);
            pLight.intensity = 1.0f;
        }

        // Render para um framebuffer
        //GLuint sceneFBO, sceneTexture;
        //Framebuffer::CreateFramebuffer(sceneFBO, sceneTexture); // você precisa implementar isso 

        // Depois da cena ter sido renderizada para sceneTexture
        //auto entity = CreateEntity();
        //AddComponent<PostProcessComponent>(entity, new Shader("postprocess.vert", "postprocess.frag"), sceneTexture, 0);

        /*Entity blurX = CreateEntity();
        AddComponent<PostProcessComponent>( blurX, new BlurEffectComponent(new Shader("post.vert", "blur.frag"), glm::vec2(1, 0)) );*/
         

        voxelSystem sys; 

        std::cout << "AAAAA" << std::endl;
        for (auto& ckt : sys.gnrtChunk()) {
            Chunk* ck = ckt.first;
            OctreeNode* node = ckt.second;

            std::cout << "o - 00" << std::endl;
            std::cout << "ck " << ck->coord.x << " " << ck->coord.y << " " << ck->coord.z << std::endl;
            auto& scene = GEngine->scene;
            auto mesh = ck->mesh.get();

            mesh->name = "CK = " +
                std::to_string(ck->coord.x) + ", " +
                std::to_string(ck->coord.y) + ", " +
                std::to_string(ck->coord.z) +
                " - lod: " + std::to_string(ck->lod) +
                "  { " + std::to_string(ck->dbg) + " }";

            Entity xz = SceneBuilder::CreateModel(ck->mesh);
            auto& pp = scene->AddComponent<SurfaceNetsComponent>(xz, ck, node, node->center);
            for (auto& nc : node->children) pp.points.push_back(nc->center);

            OctreeNode* aux = node->children[0];
            while (aux->subdivided) aux = aux->children[0];
            aux = aux->father;
            for (auto& nc : aux->children) pp.pointsDeep.push_back(nc->center);
            pp.lod = aux->depthLOD;

            //GeneratorMap::DebugPrintSDF(ck->buff->density, sysv.get_voxelGrid());

            //auto& bb = scene->GetComponent<MeshRenderer>(xz); 
            auto& aaaa = AddComponent<DebugMeshComponent>(xz);
        }
        std::cout << "AAAAAee" << std::endl;
         
        DebugRenderer::Point(glm::vec3(1, 1, 1), glm::vec3(1.0f), 6.0f, DebugPointType::Square, true);
        DebugRenderer::Point(glm::vec3(1, 2, 1), glm::vec3(1.0f), 6.0f, DebugPointType::Square, true);
        DebugRenderer::Point(glm::vec3(0, 0, 0), glm::vec3(1.0f, 0, 0), 6.0f, DebugPointType::Circle, true);
         
        {
            auto e = SceneBuilder::CreateGameObject("Font");
            auto& tr = scn->GetComponent<TransformComponent>(e);
            tr.SetLocalPosition(glm::vec3(0, 0, 0)); // topo centro (0..1)
            //tr.SetLocalPosition(glm::vec3( 0.5f, 0.5f, 0 )); // topo centro (0..1)

            auto& txt = scn->AddComponent<TextComponent>(e);
            txt.text = "DEBUG SDF VIEW";

            txt.fontID = FontManager::Load("Assets/Fonts/ProggyClean.ttf", 48);

            txt.space = TextComponent::Space::Screen;
            txt.units = TextComponent::Units::Relative;
            txt.align = TextComponent::Align::Left;
            txt.anchor = TextComponent::Anchor::TopCenter;

            txt.color = { 1,0,0,1 };

            txt.style.outlineThickness = 0.1f;
            txt.style.outlineColor = { 0,0,0,1 };

        } 
        {
            auto e = SceneBuilder::CreateGameObject("Font");
            auto& tr = scn->GetComponent<TransformComponent>(e);
            tr.SetLocalPosition(glm::vec3(0.5f, 5.5f, 0)); // topo centro (0..1)

            auto& txt = scn->AddComponent<TextComponent>(e);
            txt.text = "3dD";

            txt.fontID = FontManager::Load("Assets/Fonts/ProggyClean.ttf", 48);

            txt.space = TextComponent::Space::World;
            txt.units = TextComponent::Units::Relative;
            txt.align = TextComponent::Align::Left;
            txt.anchor = TextComponent::Anchor::TopCenter;

            txt.color = { 1,0,0,1 };

            txt.style.outlineThickness = 0.1f;
            txt.style.outlineColor = { 0,0,0,1 };

        } 

        /*
        {
            Entity e = scn->CreateEntity();

            auto& tr = scn->AddComponent<TransformComponent>(e);
            tr.position = { 0.02f, 0.85f, 0 };

            auto& txt = scn->AddComponent<TextComponent>(e);
            txt.text =
                "Voxel Debug\n"
                "LOD: 3\n"
                "Nodes: 128";

            txt.fontID = FontManager::Load("Assets/Fonts/Roboto_Condensed-Black.ttf", 24);

            txt.space = TextComponent::Space::Screen;
            txt.units = TextComponent::Units::Relative;
            //txt.align = TextComponent::Align::Left;
            txt.anchor = TextComponent::Anchor::TopLeft;

            txt.style.shadowOffset = { 2, -2 };
            txt.style.shadowColor = { 0,0,0,0.6f };


        }
        */
        /*
        {
            Entity e = scn->CreateEntity();

            auto& tr = scn->AddComponent<TransformComponent>(e);
            tr.position = { 400, 400, 0 };

            auto& txt = scn->AddComponent<TextComponent>(e);
            txt.text = "2323v";

            txt.fontID = FontManager::Load("Assets/Fonts/Roboto_Condensed-Black.ttf", 24);

            txt.space = TextComponent::Space::Screen;
            txt.units = TextComponent::Units::Pixels;
            txt.align = TextComponent::Align::Left;
            //txt.anchor = TextComponent::Anchor::TopLeft;

            txt.style.shadowOffset = { 2, -2 };
            txt.style.shadowColor = { 0,0,0,0.6f };

        }
        */


        //auto e = scn->CreateEntity();
        //auto& t = scn->AddComponent<TextComponent>(e);

        //t.text = "(C) Gear Sauce";
        //t.color = { 0.3f, 0.7f, 0.9f, 1.0f };
        //t.scale = 0.5f;
        //t.space = TextComponent::Space::Screen;
        //t.align = TextComponent::Align::Left;
        //t.fontID = FontManager::Load("fonts/Antonio-Bold.ttf", 48);

        //auto& tr = scn->AddComponent<TransformComponent>(e);
        //tr.position = { 540.0f, 570.0f, 0.0f }; // igual LearnOpenGL


        std::cout << "Corners 27" << std::endl;
        return;

    }

};
#pragma once
#include "SceneECS.h" 
#include "../Graphics/Framebuffer.h" 

#include "../ECS/Components/CameraComponent.h"
#include "../ECS/Components/TransformComponent.h"
#include "../ECS/Components/ComputeSyncComponent.h"
#include "../ECS/Components/LightComponent.h"

#include <stdexcept> 
#include "SceneBuilder.h"
#include <memory>

#include"../ECS/Systems/DebugRenderer.h"
#include "../GUI/FontManager.h" 

//#include "../Geometry/World/SurfaceNet/World/SurfaceNets/Voxel.h"
#include "../Resources/Primitives/Primitive.h"
  
//#include "../Geometry/World/DC/DCTree.h"
#include "../Geometry/Voxel/DC/DCMeshBuilder.h"
#include "../Geometry/Voxel/Octree/OctreeBuilder.h"

#include "../Geometry/World/clipmap_system.cpp"
#include "../Geometry/World/ThreadWorker.h"


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

        /*{
            stbi_set_flip_vertically_on_load(true);  
            Entity entity = SceneBuilder::CreateModel("Engine/Resources/Models/backpack/backpack.obj");  
            GetComponent<TransformComponent>(entity).position = glm::vec3(1, 1, 0);  
        } */

        {
            {
                Entity entity = SceneBuilder::CreateModel("Assets/Models3D/CapsuleP.obj");  
                auto& t = GetComponent<TransformComponent>(entity); 
                glm::mat4 model(1);
                model = glm::translate(model, glm::vec3(-18.086, -16.381, 21.492));
                model = glm::rotate(model, 32.668f, glm::vec3(0, 1, 0));
                model = glm::scale(model, glm::vec3(0.760, 0.760, 0.760));
                t.SetLocalFromMatrix(model);
            }  
            {
                Entity entity = SceneBuilder::CreateModel("Assets/Models3D/Wheel.obj");
                auto& t = GetComponent<TransformComponent>(entity); 
                // posição
                t.SetLocalPosition({ 1.089f, -10.667f, 13.343f }); 
                // rotação (Euler em graus)
                t.SetLocalEulerDegrees({ -45.308f, 65.636f, -34.439f }); 
                // escala
                t.SetLocalScale({ 1.268f, 1.268f, 1.268f }); 
                // força atualização imediata (opcional se o sistema já atualiza)
                //t.UpdateWorldAsRoot();

            } 
        }

        {
            Entity pointLight = SceneBuilder::CreateGameObject("Sun");
            auto& pTransform = GetComponent<TransformComponent>(pointLight); 
            pTransform.SetLocalRotation(glm::vec3(-155.551, 36.502, 110.245));
            auto& pLight = AddComponent<LightComponent>(pointLight); 
            pLight.SetTypeLight(LightType::Directional);
            pLight.color = glm::vec3(0.749, 0.400, 0.059);
            pLight.intensity = 4.200f;
        }
        {
            Entity pointLight = SceneBuilder::CreateGameObject("Light1");
            auto& pTransform = GetComponent<TransformComponent>(pointLight);
            pTransform.rotation = glm::vec3(-2.0f, 4.0f, -1.0f); 
            pTransform.scale = glm::vec3(0.05, 0.05, 0.05);
            pTransform.position = glm::vec3(0, 0, 0);
            auto& pLight = AddComponent<LightComponent>(pointLight); 
            pLight.SetTypeLight(LightType::Point);
            pLight.color = glm::vec3(0, 1, 0);
            pLight.intensity = 1.0f;
        }
        {
            Entity pointLight = SceneBuilder::CreateGameObject("Light2");
            auto& pTransform = GetComponent<TransformComponent>(pointLight); 
            pTransform.scale = glm::vec3(0.05, 0.05, 0.05);
            pTransform.position = glm::vec3(0, 0, 1);
            auto& pLight = AddComponent<LightComponent>(pointLight);
            pLight.SetTypeLight(LightType::Point);
            pLight.color = glm::vec3(1, 1, 1);
            pLight.intensity = 11.0f;
        }


        // Render para um framebuffer
        //GLuint sceneFBO, sceneTexture;
        //Framebuffer::CreateFramebuffer(sceneFBO, sceneTexture); // você precisa implementar isso 

        // Depois da cena ter sido renderizada para sceneTexture
        //auto entity = CreateEntity();
        //AddComponent<PostProcessComponent>(entity, new Shader("postprocess.vert", "postprocess.frag"), sceneTexture, 0);

        /*Entity blurX = CreateEntity();
        AddComponent<PostProcessComponent>( blurX, new BlurEffectComponent(new Shader("post.vert", "blur.frag"), glm::vec2(1, 0)) );*/

        
        /*
        //      DUAL CONTOURING

        //const int octreeSize = 64; // 64    //128    //256
        const int octreeSize = 256;  //pow(2, 8)  
        VertexBuffer vertexBuffer;      IndexBuffer indexBuffer;

        auto start = std::chrono::high_resolution_clock::now();

        auto* root = BuildOctree(glm::ivec3(-octreeSize / 2), octreeSize);  //, 1
        GenerateMeshFromOctree(root, vertexBuffer, indexBuffer);


        auto end = std::chrono::high_resolution_clock::now();

        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        //double minutes = ms / 60000.0;

        printf("\n\n - __WORLD_DEBUGS__ \n");
        printf("World generation took %.2f ms\n", ms);

        double totalSeconds = ms / 1000.0;
        int minutesPart = (int)(totalSeconds / 60);
        double secondsPart = fmod(totalSeconds, 60.0);

        //printf("World generation took %d minutes and %.2f seconds\n", minutesPart, secondsPart);

        printf("World generation took %d min %d s %d ms\n",
            minutesPart,
            (int)secondsPart,
            (int)((secondsPart - (int)secondsPart) * 1000));

        printf("\n\n\n");

        std::shared_ptr<MeshAsset> mesh = make_shared<MeshAsset>(vertexBuffer, indexBuffer);
        mesh->name = "DC";

        auto asset = std::make_shared<MaterialAsset>();
        asset->base = MaterialLibrary::Get("PBR_Default");
        asset->defaults["Albedo"].data = TextureCache::Get().GetSolidColor(glm::vec4(0.835f, 0.353f, 0.149f, 1.000f));
        asset->defaults["Metallic"].data = TextureCache::Get().GetSolidColor(glm::vec4(0, 0, 0, 1));
        asset->defaults["Roughness"].data = TextureCache::Get().GetSolidColor(glm::vec4(1, 1, 1, 1));

        asset->name = "DC Mat";  // name 
        auto e = SceneBuilder::CreateModel(mesh, MaterialAsset::Instantiate(asset));

        auto* t = GEngine->scene->TryGetComponent<TransformComponent>(e);
        t->SetLocalPosition(glm::vec3(0, -16, 0));
        */
         
        ClipmapSystem* system = new ClipmapSystem();
        std::cout << "\n\nlp1\n\n";
        system->Initialize();
        std::cout << "\n\nlp2\n\n";

        system->Update(glm::vec3(0));
        std::cout << "\n\nlp3\n\n";


        std::cout << "bb1" << std::endl;
        // esperar todas as tarefas do pool terminarem
        gThreadPool.WaitAll(); // você precisa implementar essa função no ThreadPool


        auto asset = std::make_shared<MaterialAsset>();
        asset->base = MaterialLibrary::Get("PBR_Default");
        asset->defaults["Albedo"].data = TextureCache::Get().GetSolidColor(glm::vec4(0.835f, 0.353f, 0.149f, 1.000f));
        asset->defaults["Metallic"].data = TextureCache::Get().GetSolidColor(glm::vec4(0, 0, 0, 1));
        asset->defaults["Roughness"].data = TextureCache::Get().GetSolidColor(glm::vec4(1, 1, 1, 1));

        asset->name = "DC Mat";  // name 
        for (auto s : system->world.chunks)
        {
            std::shared_ptr<MeshAsset> mesh = make_shared<MeshAsset>(s.second->vertexBuffer, s.second->indexBuffer);
            mesh->name = "DC";
             
            auto e = SceneBuilder::CreateModel(mesh, MaterialAsset::Instantiate(asset));

            auto* t = GEngine->scene->TryGetComponent<TransformComponent>(e);
            t->SetLocalPosition(glm::vec3(0, -16, 0));
        }

        std::cout << "bb2" << std::endl;
        //-------------------------------------------------------------------------------------------------------------------------



        int nrRows = 7;
        int nrColumns = 7;
        float spacing = 2.5;
        // render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
        glm::mat4 model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        { 
            for (int col = 0; col < nrColumns; ++col)
            {
                auto asset = std::make_shared<MaterialAsset>();
                asset->name = "Sphere";  // name 
                asset->base = MaterialLibrary::Get("PBR_Default"); 

                asset->defaults["Albedo"].data = TextureCache::Get().GetSolidColor(glm::vec4(1, 0, 0, 1));

                float r = (float)row / (float)nrRows;
                asset->defaults["Metallic"].data = TextureCache::Get().GetSolidColor(glm::vec4(r, r, r, 1));

                // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
                // on direct lighting. 
                r = glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f);
                asset->defaults["Roughness"].data = TextureCache::Get().GetSolidColor(glm::vec4(r, r, r, 1));


                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0, 4, 0));
                model = glm::translate(model, glm::vec3(
                    (float)(col - (nrColumns / 2)) * spacing,
                    (float)(row - (nrRows / 2)) * spacing,
                    -2.0f
                ));
                auto e = SceneBuilder::CreateModel(PrimitiveMesh::Sphere(), asset->Instantiate(asset));
                auto& t = SceneECS::GetComponent<TransformComponent>(e);
                //t.SetLocalFromMatrix(model);
                t.SetLocalPosition(glm::vec3(
                    (float)(col - (nrColumns / 2))* spacing,
                    (float)(row - (nrRows / 2))* spacing,
                    -2.0f));
                
            }
        }


        // render light source (simply re-render sphere at light positions)
        // this looks a bit off as we use the same shader, but it'll make their positions obvious and 
        // keeps the codeprint small.
        /*for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            newPos = lightPositions[i];
            pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

            model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            pbrShader.setMat4("model", model);
            pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            renderSphere();
        }*/ 

        
        {
            Entity e = scn->CreateEntity(); 
            auto& tr = scn->AddComponent<TransformComponent>(e);
            scn->AddComponent<NameComponent>(e).name = "TEXT 1";
            tr.SetLocalPosition( glm::vec3(0.02f, 0.026f, 0 ));

            auto& txt = scn->AddComponent<TextComponent>(e);
            txt.text =
                "Dual Contouring\n";

            txt.fontID = FontManager::Load("Assets/Fonts/Roboto_Condensed-Black.ttf", 48); 
            txt.space = TextComponent::Space::Screen;
            txt.units = TextComponent::Units::Relative;
            //txt.align = TextComponent::Align::Left;
            txt.anchor = TextComponent::Anchor::TopLeft;
            txt.color = vec4(2.5f, 0.0f, 0.0f, 1);

            txt.style.shadowOffset = { 2, -2 };
            txt.style.shadowColor = { 0,0,0,0.6f }; 
        }  

        {
            Entity e = scn->CreateEntity();

            auto& tr = scn->AddComponent<TransformComponent>(e);
            scn->AddComponent<NameComponent>(e).name = "Hello World!!!"; 
            tr.SetLocalPosition(glm::vec3(0, 11.5f, 0));

            auto& txt = scn->AddComponent<TextComponent>(e);
            txt.text = "Hello World";

            txt.scale = 0.02;
            txt.fontID = FontManager::Load("Assets/Fonts/Roboto_Condensed-Black.ttf", 128);

            txt.space = TextComponent::Space::World;
            txt.units = TextComponent::Units::Pixels;
            txt.align = TextComponent::Align::Center;
            txt.anchor = TextComponent::Anchor::Center;
            txt.color = vec4(0.75f, 0.75f, 0, 1);
            txt.style.shadowOffset = { 1.5, -1.5 };
            txt.style.shadowColor = { 0,0,0,0.6f }; 
        } 


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


    void f() { 
        //voxelSystem sys;

        //std::cout << "AAAAA" << std::endl;
        //for (auto& ckt : sys.gnrtChunk()) {
        //    Chunk* ck = ckt.first;
        //    OctreeNode* node = ckt.second;

        //    std::cout << "o - 00" << std::endl;
        //    std::cout << "ck " << ck->coord.x << " " << ck->coord.y << " " << ck->coord.z << std::endl;
        //    auto& scene = GEngine->scene;
        //    auto mesh = ck->mesh.get();

        //    mesh->name = "CK = " +
        //        std::to_string(ck->coord.x) + ", " +
        //        std::to_string(ck->coord.y) + ", " +
        //        std::to_string(ck->coord.z) +
        //        " - lod: " + std::to_string(ck->lod) +
        //        "  { " + std::to_string(ck->dbg) + " }";

        //    Entity xz = SceneBuilder::CreateModel(ck->mesh);
        //    auto& pp = scene->AddComponent<SurfaceNetsComponent>(xz, ck, node, node->center);
        //    for (auto& nc : node->children) pp.points.push_back(nc->center);

        //    OctreeNode* aux = node->children[0];
        //    while (aux->subdivided) aux = aux->children[0];
        //    aux = aux->father;
        //    for (auto& nc : aux->children) pp.pointsDeep.push_back(nc->center);
        //    pp.lod = aux->depthLOD;

        //    //GeneratorMap::DebugPrintSDF(ck->buff->density, sysv.get_voxelGrid());

        //    //auto& bb = scene->GetComponent<MeshRenderer>(xz); 
        //    auto& aaaa = AddComponent<DebugMeshComponent>(xz);
        //}
        //std::cout << "AAAAAee" << std::endl;

        //DebugRenderer::Point(glm::vec3(1, 1, 1), glm::vec3(1.0f), 6.0f, DebugPointType::Square, true);
        //DebugRenderer::Point(glm::vec3(1, 2, 1), glm::vec3(1.0f), 6.0f, DebugPointType::Square, true);
        //DebugRenderer::Point(glm::vec3(0, 0, 0), glm::vec3(1.0f, 0, 0), 6.0f, DebugPointType::Circle, true);

        //{
        //    auto e = SceneBuilder::CreateGameObject("Font");
        //    auto& tr = scn->GetComponent<TransformComponent>(e);
        //    tr.SetLocalPosition(glm::vec3(0, 0, 0)); // topo centro (0..1)
        //    //tr.SetLocalPosition(glm::vec3( 0.5f, 0.5f, 0 )); // topo centro (0..1)

        //    auto& txt = scn->AddComponent<TextComponent>(e);
        //    txt.text = "DEBUG SDF VIEW";

        //    txt.fontID = FontManager::Load("Assets/Fonts/ProggyClean.ttf", 48);

        //    txt.space = TextComponent::Space::Screen;
        //    txt.units = TextComponent::Units::Relative;
        //    txt.align = TextComponent::Align::Left;
        //    txt.anchor = TextComponent::Anchor::TopCenter;

        //    txt.color = { 1,0,0,1 };

        //    txt.style.outlineThickness = 0.1f;
        //    txt.style.outlineColor = { 0,0,0,1 };

        //}
        //{
        //    auto e = SceneBuilder::CreateGameObject("Font");
        //    auto& tr = scn->GetComponent<TransformComponent>(e);
        //    tr.SetLocalPosition(glm::vec3(0.5f, 5.5f, 0)); // topo centro (0..1)

        //    auto& txt = scn->AddComponent<TextComponent>(e);
        //    txt.text = "3dD";

        //    txt.fontID = FontManager::Load("Assets/Fonts/ProggyClean.ttf", 48);

        //    txt.space = TextComponent::Space::World;
        //    txt.units = TextComponent::Units::Relative;
        //    txt.align = TextComponent::Align::Left;
        //    txt.anchor = TextComponent::Anchor::TopCenter;

        //    txt.color = { 1,0,0,1 };

        //    txt.style.outlineThickness = 0.1f;
        //    txt.style.outlineColor = { 0,0,0,1 };

        //}

    }


};
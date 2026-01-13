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
            l.position = glm::vec3(1,1,0);
            //stbi_set_flip_vertically_on_load(false);
            //Entity entity23 = SceneBuilder::CreateModel("Resources/Models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
            //stbi_set_flip_vertically_on_load(true);
            
            /*auto& trans = GetComponent<TransformComponent>(entity);
            trans.position = { 0, 0, 0 }; 
            auto& renderer = AddComponent<MeshRenderer>(entity); 
             */ 

            //auto* material = new Material();                    //auto* material = new Material(defaultShader);  
            //auto* tex = MaterialDefaults::TextureColor(250, 100, 0);
            //tex->unit = 0;  
            //material->textures["Albedo"] = tex;
            //Entity entity2 = SceneBuilder::CreateModel(PrimitiveMesh::CreateCube(nullptr));

            //stbi_set_flip_vertically_on_load(false); 
            //entity = SceneBuilder::CreateModel("Resources/Models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
            /*auto& trans2 = GetComponent<TransformComponent>(entity);
            trans2.position = { 0, -4, 0 };
            trans2.scale = { 0.2, 0.2, 0.2 };
            trans2.rotation = { 90 , 0.2, 0.2 };  */

            //stbi_set_flip_vertically_on_load(true);

            //Entity Floor = CreateEntity();
            //auto& Floort = AddComponent<TransformComponent>(Floor);
            //Floort.position = { 0, -4, 0 };
            //Floort.scale = { 10, 10, 10 }; 
            //auto& Floort3 = AddComponent<MeshRenderer>(Floor, PrimitiveMesh::CreatePlane()); 
        }   

        {
            Entity pointLight = SceneBuilder::CreateGameObject("Sun");
            auto& pTransform = GetComponent<TransformComponent>(pointLight);
            //pTransform.rotation = glm::vec3(20.0f, 50, 20.0f);
            pTransform.rotation = glm::vec3(21.0f, 0.0f, 1.0f);
            /*auto& redn = AddComponent<MeshRenderer>(pointLight);
            redn.model = new Model("Resources/Models/backpack/backpack.obj");*/
            pTransform.scale = glm::vec3(0.05, 0.05, 0.05);
            pTransform.position = glm::vec3(20.0f, 50, 20.0f);
            auto& pLight = AddComponent<LightComponent>(pointLight);
            //pLight.type = ShadowType::Point;
            pLight.SetTypeLight(LightType::Directional);
            pLight.color = glm::vec3(1, 0, 1);
            pLight.intensity = 1.0f;
        }
        {
            Entity pointLight = SceneBuilder::CreateGameObject("Light2");
            auto& pTransform = GetComponent<TransformComponent>(pointLight);
            pTransform.rotation = glm::vec3(-2.0f, 4.0f, -1.0f);
            /*auto& redn = AddComponent<MeshRenderer>(pointLight);
            redn.model = new Model("Resources/Models/backpack/backpack.obj");*/
            pTransform.scale = glm::vec3(0.05, 0.05, 0.05);
            pTransform.position = glm::vec3(0, 0, 0);
            auto& pLight = AddComponent<LightComponent>(pointLight);
            //pLight.type = ShadowType::Point;
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



        //auto entitytt = CreateEntity();                //auto entity = scene->CreateEntity();
        //AddComponent<NativeScriptComponent>(entitytt).Bind([]() {             // scene->AddComponent<NativeScriptComponent>(entity).Bind([]() {
        //    return new PlayerCamera();
        //});
        //  
        //LoadScene2(); 

        //World
         
        //const float radius = 10.0f;

        // Cria o SDF
        //auto sdfVec = GeneratorMap::GenerateSphereSDF(dim, radius);
         

        // Prepara grid + buffer
        //buffer.grid = std::make_unique<VoxelGrid>();           // precisa de <memory>
        //buffer.grid->density = std::move(sdfVec);
        //buffer.grid->density = std::move(sdfVec);
         
        //GeneratorMap::DebugPrintSDF(GeneratorMap::GenerateSphereSDF(32, 10), 32 + 0);

        // Gera mesh (retorna atual Mesh* se quiser)
        //Mesh* mesh = SurfaceNetsCPU::Generate(*buffer->grid.get(), *params, *buffer); 




        //return;

        voxelSystem sys;
        std::cout << "o - 7" << std::endl;

        //std::shared_ptr<MaterialInstance> material = std::make_shared<MaterialInstance>(std::make_shared<PBRMaterial>()); // ou carregue o material desejado aqui
        //material->SetFallbackColor("Albedo", glm::vec3(0, 0.5f, 0.2f));
        //material->SetFallbackFloat("Roughness", 0.65f);
        //material->SetFloat("Metallic", 0.1f);
         
        std::cout << "AAAAA" << std::endl;
        for(auto& ckt : sys.gnrtChunk()) {
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
            pp.lod =  aux->depthLOD;

            //GeneratorMap::DebugPrintSDF(ck->buff->density, sysv.get_voxelGrid());

            //auto& bb = scene->GetComponent<MeshRenderer>(xz); 
            auto& aaaa = AddComponent<DebugMeshComponent>(xz);  
        } 
        std::cout << "AAAAAee" << std::endl;


        DebugRenderer::Point(glm::vec3(1, 1, 1), glm::vec3(1.0f), 6.0f, DebugPointType::Square, true);
        DebugRenderer::Point(glm::vec3(1, 2, 1), glm::vec3(1.0f), 6.0f, DebugPointType::Square, true);
        DebugRenderer::Point(glm::vec3(0, 0, 0), glm::vec3(1.0f,0,0), 6.0f, DebugPointType::Circle, true);


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
        {
            //std::cout << "Corners 1" << std::endl;
            //Mesh* mesh = SurfaceNetsGPU::Generate(*buffer->grid.get(), *params, *buffer, computeShader.ID);
            //std::cout << "Corners 2" << std::endl;

            //std::cout << "Vertices: " << buffer->positions.size() << "\n";
            //std::cout << "Indices:  " << buffer->indices.size() << "\n";


            //GeneratorMap::DebugPrintSDF(buffer->grid.get()->density, 32 + 0);

            //Entity xz = SceneBuilder::CreateModel(mesh);
            //auto& pp = AddComponent<SurfaceNetsComponent>(xz);
            //pp.buffer = buffer;
            //pp.params = params;
            //auto& aaaa = AddComponent<DebugMeshComponent>(xz);

            //DebugRenderer::AddPoint(glm::vec3(1, 1, 1), glm::vec3(1.0f), 6.0f, DebugPointType::Square, true);
            //DebugRenderer::AddPoint(glm::vec3(1, 2, 1), glm::vec3(1.0f), 6.0f, DebugPointType::Square, true);

            ///*SurfaceNetsComponent snc;
            //snc.buffer = buffer;
            //snc.params = params;
            //AddComponent<SurfaceNetsComponent>(xz, std::move(snc));*/


            //auto& corner = buffer->debug_corners;
            //std::cout << "Corners count: " << corner.size() << std::endl;
            //if (!corner.empty()) { DebugRenderer::pts = std::vector<glm::vec3>(corner.begin(), corner.end()); }
        }

    }


















    /*
    // Código ECS equivalente ao renderScene()
    void LoadScene2()
    {
        //Material* material = nullptr; // ou carregue o material desejado aqui
        //auto* material = new Material();                    //auto* material = new Material(defaultShader);
        //material->textures["Albedo"] = MaterialDefaults::TextureColor(250, 100, 0) ;

        // Cubo invertido (sala)
        //{
        //    Entity room = CreateEntity();
        //    auto& trans = AddComponent<TransformComponent>(room);
        //    trans.position = glm::vec3(0.0f);
        //    trans.scale = glm::vec3(5.0f);
        //    trans.rotation = glm::vec3(0.0f); // sem rotação

        //    auto& renderer = AddComponent<MeshRenderer>(room);
        //    renderer.model = PrimitiveMesh::CreateInverseCube(material);
        //}

        {
            Entity room = CreateEntity();
            auto& trans = AddComponent<TransformComponent>(room);
            trans.position = glm::vec3(0.0f, -2, 0);
            trans.scale = glm::vec3(200.0f);
            trans.rotation = glm::vec3(0.0f); // sem rotação  
            auto& renderer = AddComponent<MeshRenderer>(room, PrimitiveMesh::CreateCylinder());
        }

        // Cubo 1
        {
            Entity cube = CreateEntity();
            auto& trans = AddComponent<TransformComponent>(cube);
            auto& tname = AddComponent<NameComponent>(cube); 
            trans.position = glm::vec3(4.0f, -3.5f, 0.0f);
            trans.scale = glm::vec3(0.5f); 
            auto& renderer = AddComponent<MeshRenderer>(cube, PrimitiveMesh::CreateCylinder());
        }

        // Cubo 2
        {
            Entity cube = CreateEntity();
            auto& trans = AddComponent<TransformComponent>(cube);
            trans.position = glm::vec3(2.0f, 3.0f, 1.0f);
            trans.scale = glm::vec3(0.75f); 
            auto& renderer = AddComponent<MeshRenderer>(cube, PrimitiveMesh::CreateCylinder());
        }

        // Cubo 3
        {
            Entity cube = CreateEntity();
            auto& trans = AddComponent<TransformComponent>(cube);
            trans.position = glm::vec3(-3.0f, -1.0f, 0.0f);
            trans.scale = glm::vec3(0.5f); 
            auto& renderer = AddComponent<MeshRenderer>(cube, PrimitiveMesh::CreateCylinder());
        }

        // Cubo 4
        {
            Entity cube = CreateEntity();
            auto& trans = AddComponent<TransformComponent>(cube);
            trans.position = glm::vec3(-1.5f, 1.0f, 1.5f);
            trans.scale = glm::vec3(0.5f); 
            auto& renderer = AddComponent<MeshRenderer>(cube, PrimitiveMesh::CreateCylinder());
        }

        // Cubo 5 (rotacionado)
        {
            Entity cube = CreateEntity();
            auto& trans = AddComponent<TransformComponent>(cube);
            trans.position = glm::vec3(-1.5f, 2.0f, -3.0f);
            trans.scale = glm::vec3(0.75f);

            // Rotação de 60 graus em torno do vetor (1, 0, 1)
            glm::vec3 axis = glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f));
            trans.rotation = glm::degrees(glm::eulerAngles(glm::angleAxis(glm::radians(60.0f), axis)));
             
            auto& renderer = AddComponent<MeshRenderer>(cube, PrimitiveMesh::CreateCylinder());
        }
    }
    */
    







































    // renders the 3D scene
    // --------------------
    //void renderScene(Shader& shader)
    //{
    //    //// room cube
    //    //glm::mat4 model = glm::mat4(1.0f);
    //    //model = glm::scale(model, glm::vec3(5.0f));
    //    //shader.setMat4("model", model);
    //    //glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
    //    //shader.setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
    //    //renderCube();
    //    //shader.setInt("reverse_normals", 0); // and of course disable it
    //    //glEnable(GL_CULL_FACE);
    //    //// cubes
    //    //model = glm::mat4(1.0f);
    //    //model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
    //    //model = glm::scale(model, glm::vec3(0.5f));
    //    //shader.setMat4("model", model);
    //    //renderCube();
    //    //model = glm::mat4(1.0f);
    //    //model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
    //    //model = glm::scale(model, glm::vec3(0.75f));
    //    //shader.setMat4("model", model);
    //    //renderCube();
    //    //model = glm::mat4(1.0f);
    //    //model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
    //    //model = glm::scale(model, glm::vec3(0.5f));
    //    //shader.setMat4("model", model);
    //    //renderCube();
    //    //model = glm::mat4(1.0f);
    //    //model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
    //    //model = glm::scale(model, glm::vec3(0.5f));
    //    //shader.setMat4("model", model);
    //    //renderCube();
    //    //model = glm::mat4(1.0f);
    //    //model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
    //    //model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    //    //model = glm::scale(model, glm::vec3(0.75f));
    //    //shader.setMat4("model", model);
    //    //renderCube();
    //}



    //void Load() override {
    //    // Criação de entidades, componentes, recursos 

    //    /*Camera* camera = new Camera(
    //        glm::vec3(0.0f, 2.0f, 5.0f),
    //        glm::vec3(0.0f, 1.0f, 0.0f)
    //    );
    //    GEngine->mainCamera = camera;*/

    //    /*Entity camEntity = CreateEntity();
    //    AddComponent<TransformComponent>(camEntity, TransformComponent( glm::vec3(0.0f, 2.0f, 5.0f),  glm::vec3(0.0f, 1.0f, 0.0f) ));
    //    AddComponent<CameraComponent>(camEntity, new Camera());*/

    //    Entity cam1 = CreateEntity();
    //    TransformComponent t = { glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f) };
    //    AddComponent<TransformComponent>(cam1, t );
    //    AddComponent<CameraComponent>(cam1, new Camera(), true); // é a principal

    //    /*Entity cam2 = scene->CreateEntity();
    //    scene->AddComponent<TransformComponent>(cam2, TransformComponent(glm::vec3(10, 2, 0)));
    //    scene->AddComponent<CameraComponent>(cam2, new Camera());*/

    //    Entity backpack = CreateEntity();
    //    AddComponent<TransformComponent>(backpack, glm::vec3(0.0f), glm::vec3(1.0f));
    //    std::unique_ptr<Model> model = std::make_unique<Model>("Resources/Models/backpack/backpack.obj");

    //    //Material material;
    //    //Shader* shader = new Shader("shaders/default.vert", "shaders/default.frag");
    //    //// Criar material e associar shader
    //    //Material material;
    //    //material.shader = shader;

    //    //material.shader = LoadShader("shaders/basic.glsl");
    //    AddComponent<MeshRenderer>(backpack, model);


    //    /*Entity light = CreateEntity();
    //    AddComponent<Light>(light, Light::Point, glm::vec3(1.0f));

    //    Entity sun = CreateEntity();
    //    AddComponent<TransformComponent>(sun, glm::vec3(-2.0f, 4.0f, -1.0f));
    //    AddComponent<LightComponent>(sun, LightComponent{
    //        .type = LightComponent::Type::Directional,
    //        .color = glm::vec3(1.0f),
    //        .intensity = 1.0f,
    //    });*/
    //    Entity light = CreateEntity();
    //    AddComponent<TransformComponent>(light, glm::vec3(-2, 4, -1));
    //    auto& lc = AddComponent<LightComponent>(light, LightComponent::Type::Directional);
    //    lc.InitializeShadowMap(1024, 1024);


    //    

    //    //lights = std::make_unique<Lighting>(); 
    //}



};

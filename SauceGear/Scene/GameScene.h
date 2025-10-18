#pragma once
#include "SceneECS.h" 
#include "../Graphics/Framebuffer.h" 
#include "../ECS/Components/ComponentsHelper.h"
#include <stdexcept> 
#include "SceneBuilder.h"
#include "../Geometry/World/SurfaceNets/Voxel.h"
#include <memory>


class GameScene : public SceneECS {
public:
     
    void Load() override {
        // --- Main Camera ---
        Entity cameraEntity = CreateEntity();
        auto& cameraTransform = AddComponent<Transform>(cameraEntity);   
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
            Entity entity = SceneBuilder::CreateModel("Resources/Models/backpack/backpack.obj");
            auto& l = GetComponent<Transform>(entity);
            l.position = glm::vec3(1,1,0);
            //stbi_set_flip_vertically_on_load(false);
            //Entity entity23 = SceneBuilder::CreateModel("Resources/Models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
            //stbi_set_flip_vertically_on_load(true);
            
            /*auto& trans = GetComponent<Transform>(entity);
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
            /*auto& trans2 = GetComponent<Transform>(entity);
            trans2.position = { 0, -4, 0 };
            trans2.scale = { 0.2, 0.2, 0.2 };
            trans2.rotation = { 90 , 0.2, 0.2 };  */

            //stbi_set_flip_vertically_on_load(true);

            //Entity Floor = CreateEntity();
            //auto& Floort = AddComponent<Transform>(Floor);
            //Floort.position = { 0, -4, 0 };
            //Floort.scale = { 10, 10, 10 }; 
            //auto& Floort3 = AddComponent<MeshRenderer>(Floor, PrimitiveMesh::CreatePlane()); 
        }   

        {
            Entity pointLight = SceneBuilder::CreateGameObject("Sun");
            auto& pTransform = GetComponent<Transform>(pointLight);
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
            //Entity pointLight = SceneBuilder::CreateGameObject("Light2");
            //auto& pTransform = GetComponent<Transform>(pointLight);
            //pTransform.rotation = glm::vec3(-2.0f, 4.0f, -1.0f);
            ///*auto& redn = AddComponent<MeshRenderer>(pointLight);
            //redn.model = new Model("Resources/Models/backpack/backpack.obj");*/
            //pTransform.scale = glm::vec3(0.05, 0.05, 0.05);
            //pTransform.position = glm::vec3(0, 0, 0);
            //auto& pLight = AddComponent<LightComponent>(pointLight);
            ////pLight.type = ShadowType::Point;
            //pLight.SetTypeLight(LightType::Point);
            //pLight.color = glm::vec3(0, 1, 0);
            //pLight.intensity = 1.0f;
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

        voxelSystem sys;
        std::cout << "o - 7" << std::endl;

        std::shared_ptr<MaterialInstance> material = std::make_shared<MaterialInstance>(std::make_shared<PBRMaterial>()); // ou carregue o material desejado aqui
        material->SetFallbackColor("Albedo", glm::vec3(0, 1, 0));
        material->SetFallbackFloat("Roughness", 1.0f);
        material->SetFloat("Metallic", 0.0f);

        for(auto& ck : sys.gnrtChunk()) {  

            std::cout << "o - 00" << std::endl;
            auto& scene = GEngine->scene;
            Entity xz = SceneBuilder::CreateModel(ck->mesh.get(), material);
            auto& pp = scene->AddComponent<SurfaceNetsComponent>(xz, ck); 
            //GeneratorMap::DebugPrintSDF(ck->buff->density, sysv.get_voxelGrid());

            auto& bb = scene->GetComponent<MeshRenderer>(xz); 
            auto& aaaa = AddComponent<DebugMeshComponent>(xz);  
        } 

        DebugRenderer::AddPoint(glm::vec3(1, 1, 1), glm::vec3(1.0f), 6.0f, DebugPointType::Square, true);
        DebugRenderer::AddPoint(glm::vec3(1, 2, 1), glm::vec3(1.0f), 6.0f, DebugPointType::Square, true);

        DebugRenderer::AddPoint(glm::vec3(0, 0, 0), glm::vec3(1.0f,0,0), 6.0f, DebugPointType::Circle, true);
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

    // Código ECS equivalente ao renderScene()
    void LoadScene2()
    {
        //Material* material = nullptr; // ou carregue o material desejado aqui
        //auto* material = new Material();                    //auto* material = new Material(defaultShader);
        //material->textures["Albedo"] = MaterialDefaults::TextureColor(250, 100, 0) ;

        // Cubo invertido (sala)
        //{
        //    Entity room = CreateEntity();
        //    auto& trans = AddComponent<Transform>(room);
        //    trans.position = glm::vec3(0.0f);
        //    trans.scale = glm::vec3(5.0f);
        //    trans.rotation = glm::vec3(0.0f); // sem rotação

        //    auto& renderer = AddComponent<MeshRenderer>(room);
        //    renderer.model = PrimitiveMesh::CreateInverseCube(material);
        //}

        {
            Entity room = CreateEntity();
            auto& trans = AddComponent<Transform>(room);
            trans.position = glm::vec3(0.0f, -2, 0);
            trans.scale = glm::vec3(200.0f);
            trans.rotation = glm::vec3(0.0f); // sem rotação  
            auto& renderer = AddComponent<MeshRenderer>(room, PrimitiveMesh::CreateCylinder());
        }

        // Cubo 1
        {
            Entity cube = CreateEntity();
            auto& trans = AddComponent<Transform>(cube);
            auto& tname = AddComponent<NameComponent>(cube); 
            trans.position = glm::vec3(4.0f, -3.5f, 0.0f);
            trans.scale = glm::vec3(0.5f); 
            auto& renderer = AddComponent<MeshRenderer>(cube, PrimitiveMesh::CreateCylinder());
        }

        // Cubo 2
        {
            Entity cube = CreateEntity();
            auto& trans = AddComponent<Transform>(cube);
            trans.position = glm::vec3(2.0f, 3.0f, 1.0f);
            trans.scale = glm::vec3(0.75f); 
            auto& renderer = AddComponent<MeshRenderer>(cube, PrimitiveMesh::CreateCylinder());
        }

        // Cubo 3
        {
            Entity cube = CreateEntity();
            auto& trans = AddComponent<Transform>(cube);
            trans.position = glm::vec3(-3.0f, -1.0f, 0.0f);
            trans.scale = glm::vec3(0.5f); 
            auto& renderer = AddComponent<MeshRenderer>(cube, PrimitiveMesh::CreateCylinder());
        }

        // Cubo 4
        {
            Entity cube = CreateEntity();
            auto& trans = AddComponent<Transform>(cube);
            trans.position = glm::vec3(-1.5f, 1.0f, 1.5f);
            trans.scale = glm::vec3(0.5f); 
            auto& renderer = AddComponent<MeshRenderer>(cube, PrimitiveMesh::CreateCylinder());
        }

        // Cubo 5 (rotacionado)
        {
            Entity cube = CreateEntity();
            auto& trans = AddComponent<Transform>(cube);
            trans.position = glm::vec3(-1.5f, 2.0f, -3.0f);
            trans.scale = glm::vec3(0.75f);

            // Rotação de 60 graus em torno do vetor (1, 0, 1)
            glm::vec3 axis = glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f));
            trans.rotation = glm::degrees(glm::eulerAngles(glm::angleAxis(glm::radians(60.0f), axis)));
             
            auto& renderer = AddComponent<MeshRenderer>(cube, PrimitiveMesh::CreateCylinder());
        }
    }



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
    //    AddComponent<Transform>(camEntity, Transform( glm::vec3(0.0f, 2.0f, 5.0f),  glm::vec3(0.0f, 1.0f, 0.0f) ));
    //    AddComponent<CameraComponent>(camEntity, new Camera());*/

    //    Entity cam1 = CreateEntity();
    //    Transform t = { glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f) };
    //    AddComponent<Transform>(cam1, t );
    //    AddComponent<CameraComponent>(cam1, new Camera(), true); // é a principal

    //    /*Entity cam2 = scene->CreateEntity();
    //    scene->AddComponent<Transform>(cam2, Transform(glm::vec3(10, 2, 0)));
    //    scene->AddComponent<CameraComponent>(cam2, new Camera());*/

    //    Entity backpack = CreateEntity();
    //    AddComponent<Transform>(backpack, glm::vec3(0.0f), glm::vec3(1.0f));
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
    //    AddComponent<Transform>(sun, glm::vec3(-2.0f, 4.0f, -1.0f));
    //    AddComponent<LightComponent>(sun, LightComponent{
    //        .type = LightComponent::Type::Directional,
    //        .color = glm::vec3(1.0f),
    //        .intensity = 1.0f,
    //    });*/
    //    Entity light = CreateEntity();
    //    AddComponent<Transform>(light, glm::vec3(-2, 4, -1));
    //    auto& lc = AddComponent<LightComponent>(light, LightComponent::Type::Directional);
    //    lc.InitializeShadowMap(1024, 1024);


    //    

    //    //lights = std::make_unique<Lighting>(); 
    //}



};

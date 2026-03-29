#pragma once  
#include "../System.h" 
#include "../../Scene/SceneECS.h" 
#include "../../Scene/SceneBuilder.h"  
#include "../../Utils/Threads/ThreadSafeQueue.h" 
#include "../../World/World.h" 
#include "../../World/ChunkStreamingBridge.h" 
 
// Sistema ECS que roda no começo do frame
class ChunkSpawnSystem : public System {
public:
     
    ChunkSpawnSystem() {
        Initialize();
    }

    void Initialize() {
        material = std::make_shared<MaterialAsset>();
        material->base = MaterialLibrary::Get("PBR_Default");
        material->defaults["Albedo"].data    = TextureCache::Get().GetSolidColor(glm::vec4(0.835f, 0.353f, 0.149f, 1.000f));
        material->defaults["Metallic"].data  = TextureCache::Get().GetSolidColor(glm::vec4(0, 0, 0, 1));
        material->defaults["Roughness"].data = TextureCache::Get().GetSolidColor(glm::vec4(1, 1, 1, 1));
        material->name = "ChunkMaterial";  // name 
    }

    void Update(float dt) override
    {
        try {
            SceneECS* scene = GEngine->scene;
             
            auto bridge = scene->TryGetResource<ChunkStreamingBridge>();
            if (!bridge) return;      

            std::shared_ptr<Chunk> chunk;

            while (bridge->readyChunks.try_pop(chunk))          //while (readyQueue->try_pop(chunk))
            {
                //bridge->removeChunks();

                /// 1 
                /* Entity e = scene->CreateEntity(); 
                scene->AddComponent<MeshRenderer>(e, chunk->vertexBuffer); 
                vec3 worldPos = vec3(chunk->coord) * float(chunk->ChunkWorldSize()); 
                scene->AddComponent<TransformComponent>(e, worldPos); */

                /// 2
                std::shared_ptr<MeshAsset> mesh = make_shared<MeshAsset>(chunk->vertexBuffer, chunk->indexBuffer);
                auto e = SceneBuilder::CreateModel(mesh, MaterialAsset::Instantiate(material));
                auto* t = GEngine->scene->TryGetComponent<TransformComponent>(e);
                t->SetLocalPosition(glm::vec3(0, -16, 0));
                mesh->name = "Chunk";
 
                //chunk->entity = e;
                chunk->state = ChunkState::Ready;


                ChunkKey key{ chunk->coord, chunk->lod };
                ObjectsChunk[key] = e;
            }

            while (bridge->removeChunks.try_pop(chunk))        
            {   
                chunk->state = ChunkState::Empty;

                ChunkKey key{ chunk->coord, chunk->lod };
                if (ObjectsChunk.find(key) != ObjectsChunk.end()) {
                    GEngine->scene->DestroyGameObject(ObjectsChunk[key]);
                }
            }

        }
        catch (const std::exception& e) {
            // Captura exceções padrão baseadas em std::exception
            std::cout << "Exceção padrão capturada: " << e.what() << std::endl;
        }
        catch (...) {
            // Captura QUALQUER outra coisa (int, string, structs, etc.)
            std::cout << "Exceção desconhecida ou não padrão capturada!" << std::endl;

            /*
            // Código que pode lançar exceções de diferentes tipos
            throw std::runtime_error("Erro crítico!");
            // Ou throw 404; // Funciona com qualquer tipo
            */
        }
    }

private:
    std::shared_ptr<MaterialAsset> material;
    std::unordered_map<ChunkKey, Entity, ChunkKeyHasher> ObjectsChunk;
};
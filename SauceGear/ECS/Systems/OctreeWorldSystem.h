#pragma once
#include "../../Core/Camera.h"
#include "../../ECS/Components/ComponentsHelper.h"
#include "../../Core/Input.h"
#include "../Geometry/WorldOctree/SurfaceNets/OctreeLOD.h"         // sua classe OctreeLOD
#include "../Geometry/World/SurfaceNets/SurfaceNetsGPU.h"    // sua função SurfaceNetsGPU::Generate
#include "../Geometry/World/SurfaceNets/GSurfPool.h"    // sua função SurfaceNetsGPU::Generate
#include "../Scene/SceneBuilder.h" // SceneBuilder::CreateModel

#include <unordered_map>

class OctreeWorldSystem : public System {
public:
    OctreeWorldSystem(const glm::vec3& center = glm::vec3(0), int rootLOD = 6, int maxChunksPerFrame = 4)
        : m_maxChunksPerFrame(maxChunksPerFrame)
    {
        octree = std::make_unique<OctreeLOD>(center, rootLOD);
        // garante que delete a octree internamente no destructor do unique_ptr
        computeShader = new ComputeShader("SurfaceNet/SurfaceNetsGPU.comp");
    }

    ~OctreeWorldSystem() override {
        delete computeShader; // libera corretamente
    }
    ComputeShader* computeShader = nullptr;

    std::vector<float> buildDenseSDF(OctreeNode* chunk) {
        SDF_Map map;

        int N = 17; // 17 por exemplo
        std::vector<float> grid(N * N * N);

        AABB region = chunk->getBounds();
        glm::vec3 size = region.max - region.min;
        glm::vec3 minCorner = chunk->center - glm::vec3(size * 0.5f);

        for (int z = 0; z < N; z++)
            for (int y = 0; y < N; y++)
                for (int x = 0; x < N; x++) {
                    glm::vec3 p = minCorner + size * glm::vec3(
                        float(x) / float(N - 1),
                        float(y) / float(N - 1),
                        float(z) / float(N - 1)
                    );

                    grid[(z * N + y) * N + x] = map.sdf->sdfDistance(p);
                }

        return grid;
    }

    bool UpdateChunk(OctreeNode* n) {
        auto map = buildDenseSDF(n);
        //auto map = makeMap.buildSDFGrid(n, octree->root);
        //ck.resizeDensityMap(); // aloca grid denso (ex: 17x17x17)

        if (!n->chunk) n->chunk = std::make_unique<Chunk>(sysv.get_voxelGrid());
        // --- cria / redimensiona chunk --- 
        Chunk& chunk = *n->chunk;
        n->b = n->getBounds();

        chunk.lod = n->depthLOD;

        chunk.buff->densityMap = map;

        // --- pega buffer GPU ---
        SurfaceNetsGPUBuffer* gpuBuf = GlobalSurfaceNetsPool::Get().Acquire();
        gpuBuf->ensureCapacity(sysv.get_voxelGrid());   //sysv.get_cellGrid()

        // --- Gera mesh ---
        chunk.mesh = SurfaceNetsGPU::Generate(
            *chunk.buff,
            n->b.min,
            computeShader->ID,
            *gpuBuf,
            false,
            sysv.get_voxelGrid(),    //get_cellGrid
            n->edge_length() / sysv.get_cellGrid() /*voxelSize*/
        );

        GlobalSurfaceNetsPool::Get().Release(gpuBuf);

        std::cout << std::endl << std::endl;

        return true;
    }
    bool f = false;

    void Update(float deltaTime) override { 
        return;

        try {  
            syso.set_camera(glm::vec3(0,0,0));   
            // 2) Atualiza octree (decide subdivisões, enfileira chunks)
            octree->Update();

            // 3) Processar alguns chunks enfileirados (limitar o trabalho por frame)
            int processed = 0;
            while (!octree->cmptChunkScheduler.empty() && processed < m_maxChunksPerFrame) {
                OctreeNode* node = octree->cmptChunkScheduler.front();
                octree->cmptChunkScheduler.pop();
                node->isEnqueued = false; // já consumimos

                // Optional: pulo rápido se nó não for chunk válido
                if (!node || !node->isChunk()) continue;

                // Chama a tua rotina de geração (GPU SurfaceNets)
                bool ok = UpdateChunk(node); // usa a função que já tens (gera n->chunk->mesh)
                if (!ok) continue;

                // Criar/atualizar entidade para esse chunk
                Entity ent = GetOrCreateEntityForNode(node);
                auto& trans = GEngine->scene->GetComponent<TransformComponent>(ent);
                auto& rend = GEngine->scene->GetComponent<MeshRenderer>(ent); // assume MeshRenderer existe
                // coloca o transform no centro do chunk
                trans.position = node->center;
                trans.scale = glm::vec3(1.0f);

                // se o chunk gerou mesh, atualiza o renderer/model
                if (node->chunk && node->chunk->mesh) {
                    // Se você tem SceneBuilder::CreateModel(mesh, material) retorna entidade nova,
                    // aqui reusa a entidade: atualiza o MeshRenderer/model pointer
                    rend.mesh = node->chunk->mesh.get(); // ou ajuste para a sua estrutura de Model/Mesh
                    // opcional: adicione um componente que guarde ponteiro para o chunk (debug / manip)
                    if (!GEngine->scene->HasComponent<SurfaceNetsComponent>(ent)) {
                        auto& comp = GEngine->scene->AddComponent<SurfaceNetsComponent>(ent, node->chunk.get());
                        //comp.buffer = node->chunk->buff.get();
                    }
                    else {
                        auto& comp = GEngine->scene->GetComponent<SurfaceNetsComponent>(ent);
                        comp.chunk = node->chunk.get();
                    }
                }

                ++processed;
            }

            // 4) Opcional: remoção de entidades cujos nós foram destruídos (garbage collect)
            //CleanupDeadNodes(); 
        }
        catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - OctreeWorldSystem] " << e.what() << "\n";
        }
    }

private:
    std::unique_ptr<OctreeLOD> octree;
    int m_maxChunksPerFrame = 2;

    // map para reusar / localizar entidade por OctreeNode*
    std::unordered_map<OctreeNode*, Entity> nodeEntityMap;

    // Se a sua Scene API tem uma forma melhor de criar MeshRenderer, troque este helper
    Entity CreateEntityForNode(OctreeNode* n) {
        Entity e = SceneBuilder::CreateGameObject("Chunk");
        auto& t = GEngine->scene->AddComponent<TransformComponent>(e);
        t.position = n->center;
        t.scale = glm::vec3(1.0f);
        GEngine->scene->AddComponent<MeshRenderer>(e); // cria renderer vazio
        return e;
    }

    Entity GetOrCreateEntityForNode(OctreeNode* n) {
        auto it = nodeEntityMap.find(n);
        if (it != nodeEntityMap.end()) {
            return it->second;
        }
        else {
            Entity e = CreateEntityForNode(n);
            nodeEntityMap[n] = e;
            return e;
        }
    }

    void CleanupDeadNodes() {
        // percorre o mapa e remove entradas onde o node deixou de ser válido (ex: foi destruído/mergeado)
        // Aqui apenas um exemplo simples: se node->chunk == nullptr E não subdividido etc -> opcionalmente apagar
        std::vector<OctreeNode*> toRemove;
        for (auto& kv : nodeEntityMap) {
            OctreeNode* n = kv.first;
            Entity ent = kv.second;
            if (!n) {
                // remove entidade também
                if (GEngine->scene->EntityExists(ent)) GEngine->scene->DestroyEntity(ent);
                toRemove.push_back(n);
                continue;
            }
            // Se nó foi removido da octree (cheque se pertence ainda por contains ao root)
            // Implementação simples: se node não está materializado e não possui chunk -> removemos entidade
            if (!n->chunk) {
                if (GEngine->scene->EntityExists(ent)) GEngine->scene->DestroyEntity(ent);
                toRemove.push_back(n);
            }
        }
        for (auto* n : toRemove) nodeEntityMap.erase(n);
    }

    // *********** Sua função UpdateChunk deve estar visível aqui ***********
    // bool UpdateChunk(OctreeNode* n);
    // eu assumo que UpdateChunk está declarada/implementada externamente no seu módulo
};

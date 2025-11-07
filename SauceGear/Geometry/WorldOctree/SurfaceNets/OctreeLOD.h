#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <iostream>
#include "../Graphics/ComputeShader.h" 
#include "../Geometry/World/SurfaceNets/SurfaceNetsGPU.h"   
#include "../Geometry/World/SurfaceNets/MapGenerator.h"    
#include "../Geometry/World/SurfaceNets/GSurfPool.h"    

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>   // (opcional, se usar ponteiros para GPU)
#include <glm/common.hpp>         // (para compMax, compMin, etc.)


// Octree Node
struct OctreeNode {
    glm::vec3 position;
    float size;
    int lodLevel = 0;
    bool subdivided = false;
    OctreeNode* parent = nullptr;
    OctreeNode* children[8] = { nullptr };

    std::unique_ptr<Chunk> chunk; // guarda SDF e Mesh

    float dbg = 0;
};

struct LODSettings {
    float baseChunkSize = 25.f ;      // Tamanho físico de um chunk LOD 0
    int maxDepth = 4;                // Quantos níveis de LOD existem
    float shellRadius = 15.f;        // Raio da região de detalhe máximo
    float detailFactor = 1.0f;       // Escala de densidade (1 = padrão, <1 = mais detalhe, >1 = menos)
};

// Octree LOD
class LODOctree {
public:
    OctreeNode* root;
    GPUMapGenerator* generator;
    ComputeShader* computeShader;
    int maxDepth;   // it's same that maxLOD 

    LODSettings settings;
    float lodDistance[4] = { 15, 30, 50, 80 }; // distância máxima antes de subdividir 

    LODOctree(GPUMapGenerator* gen, ComputeShader* compute, glm::vec3 worldCenter, float worldSize, int maxDepth = 3) {
        computeShader = compute;
        generator = gen;
        root = new OctreeNode{ worldCenter, worldSize, maxDepth };  //0


        std::vector<float> lodRatio = { 0.1f, 0.25f, 0.5f, 1.0f }; // relativo ao tamanho
        for (int i = 0; i <= maxDepth; i++) lodDistance[i] = worldSize * lodRatio[i];

    }

    int ComputeLODLevel(const glm::vec3& cameraPos, const glm::vec3& nodeCenter, const LODSettings& settings)
    {
        float dist = glm::compMax(glm::abs(cameraPos - nodeCenter)); // norma infinito
        float normalized = dist / (2.0f * settings.shellRadius * settings.detailFactor);
        float lod = std::log2(std::max(1.0f, normalized));
        return glm::clamp((int)std::ceil(lod), 0, settings.maxDepth - 1);
    }

    void UpdateLOD(const glm::vec3& cameraPos)
    {
        std::queue<OctreeNode*> q;
        q.push(root);

        while (!q.empty()) {
            OctreeNode* node = q.front(); q.pop();

            int targetLOD = ComputeLODLevel(cameraPos, node->position, settings);

            if (node->lodLevel > targetLOD && node->lodLevel > 0) {
                // Muito perto -> subdividir
                if (!node->subdivided) Subdivide(node);
            }
            else if (node->lodLevel < targetLOD) {
                // Muito longe -> mesclar
                if (node->subdivided) Merge(node);
            }

            if (node->subdivided) {
                for (int i = 0; i < 8; i++) q.push(node->children[i]);
            }

        }

        std::cout << "\n=== LOD UPDATE START ===" << std::endl;
        std::cout << "Camera: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")\n";
        PrintOctree(root);
        std::cout << "=== LOD UPDATE END ===\n" << std::endl;

        // ---- Passo 2: Gera chunks apenas nas folhas ----
        GenerateLeafChunks(root);
    }

    void UpdateLOD2(const glm::vec3& camPos) {
        std::cout << "    " << std::endl;
        // ---- Passo 1: Atualiza estrutura da árvore ----
        std::queue<OctreeNode*> q;
        q.push(root);

        while (!q.empty()) {
            OctreeNode* node = q.front(); q.pop();
            float dist = glm::distance(camPos, node->position);
            node->dbg = dist;

            std::cout << "node LOD" << node->lodLevel << std::endl;
            std::cout << "verificacao " << (node->lodLevel > 0 && dist < lodDistance[node->lodLevel]) << " - " << " lodLevel > 0 = " << to_string(node->lodLevel > 0) << " - lodDistance " << std::to_string(dist < lodDistance[node->lodLevel]) << std::endl;
            std::cout << "node pos" << node->position.x << " " << node->position.y << " " << node->position.z << std::endl;
            std::cout << "node size" << node->size << std::endl;
            std::cout << "dist " << dist << std::endl;


            // Subdivide se está perto e ainda não no LOD máximo
            if (node->lodLevel > 0 && dist < lodDistance[node->lodLevel]) {    //maxLOD == 0    maxDepth decrement until 0 increasing lod details   
                if (!node->subdivided)
                    Subdivide(node);

                std::cout << "subdivided" << std::endl;
            }
            else {
                // Merge se está longe
                if (node->subdivided) Merge(node);

                std::cout << "Merged" << std::endl;
            }

            if (node->subdivided) {
                for (int i = 0; i < 8; i++)
                    q.push(node->children[i]);
            }
            std::cout << "    " << std::endl;
        }


        std::cout << "\n=== LOD UPDATE START ===" << std::endl;
        std::cout << "Camera: (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")\n";
        PrintOctree(root);
        std::cout << "=== LOD UPDATE END ===\n" << std::endl;




        // ---- Passo 2: Gera chunks apenas nas folhas ----
        GenerateLeafChunks(root);
    }

    void PrintOctree(OctreeNode* node, int depth = 0)
    {
        if (!node) return;

        std::string indent(depth * 2, ' ');
        std::string status = node->subdivided ? "[+]" : "[ ]";

        std::cout
            << indent
            << status << " LOD " << node->lodLevel
            << " size=" << node->size
            << " pos=(" << node->position.x << ", " << node->position.y << ", " << node->position.z << ")"
            << " dist=" << node->dbg
            << (node->subdivided ? " (subdivided)" : " (leaf)")
            << std::endl;

        if (node->subdivided) {
            for (int i = 0; i < 8; i++)
                PrintOctree(node->children[i], depth + 1);
        }
    }


    void GenerateLeafChunks(OctreeNode* node) {
        if (!node) return;

        if (node->subdivided) {
            for (int i = 0; i < 8; i++)
                GenerateLeafChunks(node->children[i]);
            return;
        }

        // Folha -> gerar mesh
        GenerateChunk(node);
    }

    // Coleta todos os chunks folha (sem gerar nada novo)
    std::vector<Chunk*> CollectLeafChunks() {
        std::vector<Chunk*> leafChunks;
        std::queue<OctreeNode*> q;
        q.push(root);

        while (!q.empty()) {
            OctreeNode* node = q.front(); q.pop();

            if (node->subdivided) {
                for (int i = 0; i < 8; i++)
                    q.push(node->children[i]);
                continue;
            }

            // nó folha → adiciona se tiver mesh válida
            if (node->chunk && node->chunk->mesh) {
                leafChunks.push_back(node->chunk.get());
            }
        }

        return leafChunks;
    }


private:
    void Subdivide(OctreeNode* node) {
        float hs = node->size * 0.5f;   // half size
        int lod = node->lodLevel - 1; // + 1 lod detail         LOD 0 - the most details, LOD == maxDepth - has less details

        for (int i = 0; i < 8; i++) {
            glm::vec3 offset(
                (i & 1 ? 0.5f : -0.5f) * hs,
                (i & 2 ? 0.5f : -0.5f) * hs,
                (i & 4 ? 0.5f : -0.5f) * hs
            );
            node->children[i] = new OctreeNode{
                node->position + offset, // position is center of child
                hs,
                lod,
                false,
                node
            };
        }
        node->subdivided = true;
    }

    void Merge(OctreeNode* node) {
        if (!node->subdivided) return;
        for (int i = 0; i < 8; i++) {
            delete node->children[i];
            node->children[i] = nullptr;
        }
        node->subdivided = false;
    }


    //int GridVoxelPerAxis = std::max(2, sysv.get_cellGrid() << node->lodLevel);
    //float voxelSize = node->size / float(GridVoxelPerAxis);

    void GenerateChunk(OctreeNode* node) {
        if (!node) return;

        // LOD-aware: calcula número de voxels e voxelSize
        int GridVoxelPerAxis = std::max(8, sysv.get_cellGrid() >> node->lodLevel); // shift right reduz com LOD        // mínimo 2

        //int GridVoxelPerAxis = std::max(2,  sysv.get_cellGrid() >> (maxLOD - node->lodLevel)  ); // shift right reduz com LOD        // mínimo 2
        //float voxelSize = node->size / float(sysv.get_cellGrid() - 1);

        // mantém resolução base, mas voxelSize ajusta com o LOD
        //int GridVoxelPerAxis = sysv.get_cellGrid();
        float voxelSize = node->size / float(GridVoxelPerAxis);     //float(GridVoxelPerAxis - 1)

        std::cout << "\n voxelSize " << voxelSize * float(GridVoxelPerAxis - 1) << std::endl;

        //int GridVoxelPerAxis = std::max(2, sysv.get_cellGrid() << node->lodLevel);
        // voxelSize é proporcional ao tamanho do chunk e grid atual
        //float voxelSize = node->size / float(GridVoxelPerAxis - 1);
        //float voxelSize = node->size / float(GridVoxelPerAxis);

        //float voxelSize = node->size / float(GridVoxelPerAxis - 1);  

        //int GridVoxelPerAxis = sysv.get_cellGrid() << node->lodLevel; // dobra a resolução a cada subdivisão 



        // cria ou redimensiona chunk
        if (!node->chunk) node->chunk = std::make_unique<Chunk>(GridVoxelPerAxis);
        auto& chunk = *node->chunk;

        // offset do canto mínimo do node 
        glm::vec3 offset = node->position - glm::vec3(node->size * 0.5f);
        //glm::vec3 offset = node->position;

        // Acquire GPU buffer do pool
        SurfaceNetsGPUBuffer* gpuBuf = GlobalSurfaceNetsPool::Get().Acquire();
        gpuBuf->ensureCapacity(GridVoxelPerAxis);

        // Gera SDF e mesh
        generator->Generate(offset, *chunk.buff, *gpuBuf, GridVoxelPerAxis, voxelSize);
        chunk.mesh = SurfaceNetsGPU::Generate(*chunk.buff, offset, computeShader->ID, *gpuBuf, true, GridVoxelPerAxis, voxelSize);

        if (chunk.mesh) {
            std::cout << "offset " << offset.x << " " << offset.y << " " << offset.z << std::endl;
            std::cout << "node->position " << node->position.x << " " << node->position.y << " " << node->position.z << std::endl;
            std::cout << "v " << voxelSize << std::endl;
            std::cout << "GridVoxelPerAxis " << GridVoxelPerAxis << std::endl;
            std::cout << "node->lodLevel " << node->lodLevel << std::endl;
            std::cout << "node->size " << node->size << std::endl;
        }

        // libera GPU buffer
        GlobalSurfaceNetsPool::Get().Release(gpuBuf);

        // Atualiza informações do chunk
        chunk.coord = offset;
        chunk.lod = node->lodLevel;

        chunk.dbg = node->dbg;
    }

};



/*
for (int i = 0; i < 8; i++)
    glm::vec3 offset(
        (i & 1 ? 0.25f : -0.25f) * node->size,
        (i & 2 ? 0.25f : -0.25f) * node->size,
        (i & 4 ? 0.25f : -0.25f) * node->size
    );

Child 0 (i=0, bits=000): (-0.5, -0.5, -0.5)
Child 1 (i=1, bits=001): (+0.5, -0.5, -0.5)
Child 2 (i=2, bits=010): (-0.5, +0.5, -0.5)
Child 3 (i=3, bits=011): (+0.5, +0.5, -0.5)
Child 4 (i=4, bits=100): (-0.5, -0.5, +0.5)
Child 5 (i=5, bits=101): (+0.5, -0.5, +0.5)
Child 6 (i=6, bits=110): (-0.5, +0.5, +0.5)
Child 7 (i=7, bits=111): (+0.5, +0.5, +0.5)

*/
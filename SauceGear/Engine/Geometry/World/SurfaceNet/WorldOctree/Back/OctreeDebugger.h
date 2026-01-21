#pragma once
#include <iostream>
#include <iomanip>
#include <queue>
#include <sstream>
#include "OctreeLOD.h"

class OctreeDebugger {
public:
    static void PrintStepByStep(LODOctree& tree, const glm::vec3& camPos) {
        ResetFlags(tree.root);
        std::cout << "\n\033[1;36m=== LOD UPDATE DEBUG START ===\033[0m\n";
        std::cout << "Camera: (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")\n\n";

        std::queue<OctreeNode*> q;
        q.push(tree.root);

        int cntSubdiv = 0, cntMerge = 0;
        while (!q.empty()) {
            OctreeNode* node = q.front(); q.pop();
            float dist = glm::distance(camPos, node->position);
            node->dbg = dist;

            bool doSubdivide = (node->lodLevel > 0 && dist < tree.lodDistance[ClampLOD(node->lodLevel)]);
            PrintNode(node);

            if (doSubdivide && !node->subdivided) {
                std::cout << "  -> \033[1;32mSUBDIVIDE\033[0m\n";
                cntSubdiv++;
            }
            else if (!doSubdivide && node->subdivided) {
                std::cout << "  -> \033[1;31mMERGE\033[0m\n";
                cntMerge++;
            }
            else {
                std::cout << "  -> (no change)\n";
            }

            if (node->subdivided)
                for (int i = 0; i < 8; ++i)
                    if (node->children[i]) q.push(node->children[i]);
        }

        std::cout << "\n\033[1;36m--- TREE STRUCTURE ---\033[0m\n";
        PrintTree(tree.root);
        std::cout << "\nSummary: subdiv=" << cntSubdiv << " merge=" << cntMerge << "\n";
        std::cout << "\033[1;36m=== LOD UPDATE DEBUG END ===\033[0m\n\n";
    }

private:
    static void PrintNode(OctreeNode* node) {
        std::ostringstream ss;
        ss << "Node LOD=" << node->lodLevel
            << " pos=(" << std::fixed << std::setprecision(1)
            << node->position.x << "," << node->position.y << "," << node->position.z << ")"
            << " size=" << node->size
            << " dist=" << std::setprecision(2) << node->dbg;
        std::cout << ss.str();
    }

    static void PrintTree(OctreeNode* node, int depth = 0) {
        /*if (!node) return;
        std::string indent(depth * 2, ' ');
        const char* GREEN = "\033[1;32m", * RESET = "\033[0m";
        std::cout << indent << (node->subdivided ? GREEN "[+]" RESET : "[ ]")
            << " LOD " << node->lodLevel
            << " dist=" << node->dbg << "\n";
        if (node->subdivided)
            for (int i = 0; i < 8; ++i)
                PrintTree(node->children[i], depth + 1);*/
    }

    static void ResetFlags(OctreeNode* n) {
        if (!n) return;
        n->changed = n->visited = false;
        for (int i = 0; i < 8; i++)
            if (n->children[i]) ResetFlags(n->children[i]);
    }

    static int ClampLOD(int idx) {
        return std::clamp(idx, 0, 7);
    }
};













/*

#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include "../Graphics/ComputeShader.h"
#include "../Geometry/World/SurfaceNets/SurfaceNetsGPU.h"
#include "../Geometry/World/SurfaceNets/MapGenerator.h"
#include "../Geometry/World/SurfaceNets/GSurfPool.h"

// -----------------------------
// Octree node (com flags de debug)
// -----------------------------
struct OctreeNode {
    glm::vec3 position;
    float size;
    int lodLevel = 0;
    bool subdivided = false;
    OctreeNode* parent = nullptr;
    OctreeNode* children[8] = { nullptr };

    std::unique_ptr<Chunk> chunk; // guarda SDF e Mesh

    // debug/state
    float dbg = 0.0f;        // distância atual para camera
    bool changed = false;    // se mudou (subdividiu/mergeou) neste frame
    bool visited = false;    // uso interno se precisar
};

// -----------------------------
// LODOctree com debug passo-a-passo
// -----------------------------
class LODOctree {
public:
    OctreeNode* root;
    GPUMapGenerator* generator;
    ComputeShader* computeShader;
    int maxDepth = 3;
    // Distâncias por nível (ajuste conforme sua cena)
    float lodDistance[8] = { 15.f, 30.f, 50.f, 80.f, 160.f, 320.f, 640.f, 1280.f };

    LODOctree(GPUMapGenerator* gen, ComputeShader* compute, glm::vec3 worldCenter, float worldSize, int maxDepth_ = 3) {
        computeShader = compute;
        generator = gen;
        maxDepth = maxDepth_;
        root = new OctreeNode{ worldCenter, worldSize, maxDepth, false, nullptr, {nullptr}, nullptr, 0.0f, false, false };
        // se quiser preencher lodDistance com função baseada em worldSize:
        // for (int i=0;i<=maxDepth;i++) lodDistance[i] = worldSize * powf(0.5f, i) * factor;

        std::vector<float> lodRatio = { 0.1f, 0.25f, 0.5f, 1.0f }; // relativo ao tamanho
        for (int i = 0; i <= maxDepth; i++) lodDistance[i] = worldSize * lodRatio[i];


    }

    ~LODOctree() {
        DeleteNodeRecursive(root);
    }

    // -----------------------------
    // UpdateLOD: imprime passo a passo e no final resumo
    // -----------------------------
    void UpdateLOD(const glm::vec3& camPos) {
        // reset flags
        ResetFlags(root);

        std::cout << "\n\033[1;36m=== LOD UPDATE (step-by-step) START ===\033[0m\n";
        std::cout << "Camera: (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")\n\n";

        // BFS para decidir subdivide/merge, imprimindo cada ação
        std::queue<OctreeNode*> q;
        q.push(root);

        // counters
        int cntSubdivided = 0;
        int cntMerged = 0;

        while (!q.empty()) {
            OctreeNode* node = q.front(); q.pop();
            float dist = glm::distance(camPos, node->position);
            node->dbg = dist;

            bool wasSubdiv = node->subdivided;
            bool doSubdivide = (node->lodLevel > 0) && (dist < lodDistance[ClampLODIndex(node->lodLevel)]);
            // note: condição usando your lodLevel semantics - se usar inverso, adapte.

            // Print current node state before change
            PrintNodeInline(node);

            if (doSubdivide && !node->subdivided) {
                Subdivide(node);
                node->changed = true;
                cntSubdivided++;
                std::cout << "  -> " << "\033[1;32mSUBDIVIDE\033[0m\n";
            }
            else if (!doSubdivide && node->subdivided) {
                Merge(node);
                node->changed = true;
                cntMerged++;
                std::cout << "  -> " << "\033[1;31mMERGE\033[0m\n";
            }
            else {
                std::cout << "  -> " << (node->subdivided ? "keep subdivided" : "keep leaf") << "\n";
            }

            // enqueue children if subdivided AFTER possible change
            if (node->subdivided) {
                for (int i = 0; i < 8; ++i) {
                    if (node->children[i]) q.push(node->children[i]);
                }
            }
        }

        // show full tree after operations (colored)
        std::cout << "\n\033[1;36m--- Tree AFTER updates ---\033[0m\n";
        PrintOctreeColored(root);
        std::cout << "\n\033[1;36m=== LOD UPDATE (summary) ===\033[0m\n";
        int totalNodes = CountNodes(root);
        int leafNodes = CountLeaves(root);
        std::cout << "Nodes total: " << totalNodes << " | Leaves: " << leafNodes
            << " | Subdivided this frame: " << cntSubdivided
            << " | Merged this frame: " << cntMerged << std::endl;
        std::cout << "\033[1;36m=== LOD UPDATE END ===\033[0m\n\n";

        // agora gera chunks nas folhas (opcional)
        GenerateLeafChunks(root);
    }

    // -----------------------------
    // Coleta folhas (sem gerar), útil para render loop
    // -----------------------------
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

            if (node->chunk && node->chunk->mesh) leafChunks.push_back(node->chunk.get());
        }
        return leafChunks;
    }

private:
    // -----------------------------
    // Util: apaga recursivamente
    // -----------------------------
    void DeleteNodeRecursive(OctreeNode* n) {
        if (!n) return;
        for (int i = 0; i < 8; i++) {
            if (n->children[i]) DeleteNodeRecursive(n->children[i]);
        }
        delete n;
    }

    // -----------------------------
    // Clamp safe para index do lodDistance
    // -----------------------------
    int ClampLODIndex(int idx) {
        if (idx < 0) return 0;
        int maxIdx = (int)(sizeof(lodDistance) / sizeof(lodDistance[0])) - 1;
        if (idx > maxIdx) return maxIdx;
        return idx;
    }

    // -----------------------------
    // Reset flags changed/visited
    // -----------------------------
    void ResetFlags(OctreeNode* n) {
        if (!n) return;
        n->changed = false;
        n->visited = false;
        for (int i = 0; i < 8; i++) if (n->children[i]) ResetFlags(n->children[i]);
    }

    // -----------------------------
    // PrintInline: linha simples para decisão (antes da mudança)
    // -----------------------------
    void PrintNodeInline(OctreeNode* node) {
        std::ostringstream ss;
        ss << "Node LOD=" << node->lodLevel << " pos=("
            << std::fixed << std::setprecision(1) << node->position.x << ","
            << node->position.y << "," << node->position.z << ")"
            << " size=" << node->size
            << " dist=" << std::setprecision(2) << node->dbg;
        std::cout << ss.str();
    }

    // -----------------------------
    // Print tree colorida (recursiva)
    // -----------------------------
    void PrintOctreeColored(OctreeNode* node, int depth = 0) {
        if (!node) return;
        const char* RESET = "\033[0m";
        const char* GREEN = "\033[1;32m";  // subdividiu
        const char* RED = "\033[1;31m";  // mergeou (ou mudou p/ leaf)
        const char* WHITE = "\033[1;37m";  // sem mudança
        const char* YELLOW = "\033[1;33m";  // leaf que mudou
        std::string indent(depth * 2, ' ');

        const char* color = WHITE;
        std::string mark = "[·]"; // default leaf mark
        if (node->subdivided) { color = GREEN; mark = "[+]"; }
        if (node->changed && node->subdivided == false) { color = YELLOW; mark = "[M]"; } // merged to leaf
        if (node->changed && node->subdivided == true) { color = GREEN; mark = "[+*]"; } // subdivided this frame

        std::cout << indent << color << mark
            << " LOD " << node->lodLevel
            << " | size=" << std::fixed << std::setprecision(2) << node->size
            << " | pos=(" << std::setprecision(1)
            << node->position.x << ", " << node->position.y << ", " << node->position.z << ")"
            << " | dist=" << std::setprecision(1) << node->dbg
            << RESET << std::endl;

        if (node->subdivided) {
            for (int i = 0; i < 8; ++i) PrintOctreeColored(node->children[i], depth + 1);
        }
    }

    // -----------------------------
    // Subdivide (marca changed)
    // -----------------------------
    void Subdivide(OctreeNode* node) {
        if (!node) return;
        if (node->subdivided) return;
        float hs = node->size * 0.5f;
        int childLOD = node->lodLevel - 1; // assume lodLevel decreases with depth (adapt if usa outro esquema)
        for (int i = 0; i < 8; i++) {
            glm::vec3 offset(
                (i & 1 ? 0.5f : -0.5f) * hs,
                (i & 2 ? 0.5f : -0.5f) * hs,
                (i & 4 ? 0.5f : -0.5f) * hs
            );
            OctreeNode* child = new OctreeNode();
            child->position = node->position + offset;
            child->size = hs;
            child->lodLevel = childLOD;
            child->parent = node;
            child->subdivided = false;
            child->dbg = 0.0f;
            child->changed = true; // new node is 'changed'
            node->children[i] = child;
        }
        node->subdivided = true;
        node->changed = true;
    }

    // -----------------------------
    // Merge (marca changed)
    // -----------------------------
    void Merge(OctreeNode* node) {
        if (!node) return;
        if (!node->subdivided) return;
        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                // optionally preserve children data to parent before delete
                DeleteNodeRecursive(node->children[i]);
                node->children[i] = nullptr;
            }
        }
        node->subdivided = false;
        node->changed = true;
    }

    // -----------------------------
    // Gera chunks apenas nas folhas (mantive sua rotina)
    // -----------------------------
    void GenerateLeafChunks(OctreeNode* node) {
        if (!node) return;
        if (node->subdivided) {
            for (int i = 0; i < 8; i++) GenerateLeafChunks(node->children[i]);
            return;
        }
        // folha: aqui você pode chamar GenerateChunk(node) se quiser gerar meshes
        // GenerateChunk(node); // descomente se desejar geração automática aqui
    }

    // -----------------------------
    // Node counters utilitários
    // -----------------------------
    int CountNodes(OctreeNode* n) {
        if (!n) return 0;
        int c = 1;
        if (n->subdivided) for (int i = 0; i < 8; i++) c += CountNodes(n->children[i]);
        return c;
    }
    int CountLeaves(OctreeNode* n) {
        if (!n) return 0;
        if (!n->subdivided) return 1;
        int c = 0;
        for (int i = 0; i < 8; i++) c += CountLeaves(n->children[i]);
        return c;
    }

    // -----------------------------
    // Se quiser gerar mesh aqui, reimplemente / chame seu GenerateChunk
    // -----------------------------
    void GenerateChunk(OctreeNode* node) {
        // Implementação deixada vazia de propósito.
        // No seu fluxo original você tem um GenerateChunk que chama GPUMapGenerator etc.
        // Se quiser, chamo aqui e atualizo node->chunk.
    }
};


*/
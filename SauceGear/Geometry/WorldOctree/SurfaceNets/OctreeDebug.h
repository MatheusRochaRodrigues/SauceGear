#pragma once 
#include <iostream>
#include <string>
#include "OctreeNode.h" 

// ==========================
// CONFIGURAÇÃO DE DEBUG
// ==========================
#define OCTREE_DEBUG 0            // 1 = ligado, 0 = desligado
#define OCTREE_DEBUG_TREE 1       // imprimir árvore depois
#define OCTREE_DEBUG_STEP 1       // printar passo a passo da Update()
#define OCTREE_DEBUG_COLOR 1      // cores ANSI ligadas

#if OCTREE_DEBUG_COLOR
#define C_RESET   "\033[0m"
#define C_RED     "\033[1;31m"
#define C_GREEN   "\033[1;32m"
#define C_YELLOW  "\033[1;33m"
#define C_BLUE    "\033[1;34m"
#define C_CYAN    "\033[1;36m"
#define C_MAGENTA "\033[1;35m"
#define C_WHITE   "\033[1;37m"
#else
#define C_RESET   ""
#define C_RED     ""
#define C_GREEN   ""
#define C_YELLOW  ""
#define C_BLUE    ""
#define C_CYAN    ""
#define C_MAGENTA ""
#define C_WHITE   ""
#endif 


// ==============================================
//   SISTEMA DE DEBUG PARA OCTREELOD (COM ANSI)
// ==============================================
class OctreeDebug {
public:

    // ==================================================
    // IMPRIME CABEÇALHO DO NÓ DURANTE A Update()
    // ==================================================
    static void PrintNodeHeader(OctreeNode* n) {
#if OCTREE_DEBUG
        std::cout << C_CYAN << "=== NODE ===============================" << C_RESET << "\n";

        std::cout << "Center: ("
            << n->center.x << ", "
            << n->center.y << ", "
            << n->center.z << ")\n";
        std::cout << "LOD depth: " << n->depthLOD << "\n";
        std::cout << "Subdivided: " << (n->subdivided ? C_GREEN "YES" : C_RED "NO") << C_RESET << "\n";

        std::cout << C_CYAN << "----------------------------------------" << C_RESET << "\n";
#endif
    }

    // ==================================================
    // IMPRIME VALOR DO SDF
    // ==================================================
    static void PrintSDF(OctreeNode* n) {
#if OCTREE_DEBUG
        std::cout << "SDF value: " << (n->distSurf_SDF < 0 ? C_RED : C_GREEN) << n->distSurf_SDF << C_RESET << "\n";
#endif
    }

    // ==================================================
    // IMPRIME A DECISÃO DE SUPERFÍCIE
    // ==================================================
    static void PrintSurfaceDecision(bool hasSurface) {
#if OCTREE_DEBUG
        std::cout << "Surface test: " << (hasSurface ? C_GREEN "YES" : C_RED "NO") << C_RESET << "\n";
#endif
    }

    // ==================================================
    // IMPRIME MATERIALIZE()
    // ==================================================
    static void PrintMaterialize(OctreeNode* n) {
#if OCTREE_DEBUG
        std::cout << C_MAGENTA << "Materialized node!" << C_RESET << "\n";
#endif
    }

    // ==================================================
    // IMPRIME QUANDO UM CHUNK É AGENDADO
    // ==================================================
    static void PrintChunkQueued() {
#if OCTREE_DEBUG
        std::cout << C_YELLOW << "[Chunk Queued]" << C_RESET << "\n";
#endif
    }

    static void Subdiveded() {
#if OCTREE_DEBUG
        std::cout << C_YELLOW << "[Subdiveded]" << C_RESET << "\n";
#endif
    }

    // ==================================================
    // PRINT SIMPLES DE NO PARA ÁRVORE
    // ==================================================
    static void PrintTreeNode(OctreeNode* n, int depth) {
#if OCTREE_DEBUG && OCTREE_DEBUG_TREE
        std::string indent(depth * 2, ' ');

        const char* color =
            n->subdivided ? C_GREEN :
            n->materialized == 0xFF ? C_MAGENTA :
            C_WHITE;

        std::cout << indent << color;

        if (n->subdivided) std::cout << "[+] ";
        else std::cout << "[ ] ";

        //std::cout << "LOD=" << n->depthLOD;

        std::cout << " LOD=" << n->depthLOD
            << " desired=" << n->desiredLOD
            << " center=(" << n->center.x << "," << n->center.y << "," << n->center.z << ")";
        if (n->isChunk()) std::cout << C_RED << " [CHUNK]" << C_RESET;

        if (n->materialized == 0xFF)
            std::cout << " <MAT>";

        std::cout << C_RESET << "\n";
#endif
    }

    // ==================================================
    // PRINT RECURSIVO DA ÁRVORE DEPOIS DA UPDATE()
    // ==================================================
    static void PrintTree(OctreeNode* n, int depth = 0) {
#if OCTREE_DEBUG && OCTREE_DEBUG_TREE
        if (!n) return;

        PrintTreeNode(n, depth);

        if (n->subdivided)
            for (auto* c : n->children)
                PrintTree(c, depth + 1);
#endif
    }




    /*static void PrintTree(const OctreeNode* node, int depth = 0)
    {
        if (!node) return;

        std::string indent(depth * 2, ' ');

        std::cout << indent;
        std::cout << (node->subdivided ? "+" : "-");
        std::cout << " LOD=" << node->depthLOD
            << " desired=" << node->desiredLOD
            << " center=(" << node->center.x << "," << node->center.y << "," << node->center.z << ")";
        if (node->chunk) std::cout << " [CHUNK]";
        std::cout << "\n";

        if (node->subdivided)
            for (int i = 0; i < 8; i++)
                PrintTree(node->children[i], depth + 1);
    }*/
};
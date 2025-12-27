#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <vector>
#include <functional> 
#include <limits>
#include <iostream>

#define sysv SysVoxel::getInstance()
#define syso SysOctree::getInstance()

struct SysVoxel { 
    glm::vec3 numChunksPerAxis = glm::vec3(5.0f, 2.0f, 5.0f); // quantos chunks criar em cada eixo  

    //for keep default singleton
    static SysVoxel& getInstance() {       //getVoxelGrid
        static SysVoxel instance; // criado na primeira chamada
        return instance;
    }
    SysVoxel(const SysVoxel&) = delete;
    SysVoxel& operator=(const SysVoxel&) = delete;


    void    set_cellGrid(int s) { cellGrid = s; }     //set_voxelGrid(int s) { cellGrid  = s; }
    void    set_base0ChunkSize(int s) { baseChunkSize0 = s; }
    int     get_Border() { return (int)border /* + 1 to border 2*/; /*resolution default = 17*/ }    //static_cast<int>()

    int     get_cellGrid()   { return cellGrid; }  // número de células  //ex : 16  
    int     get_voxelGrid()  { return cellGrid + 1; /*resolution default = 17*/ } // nº de pontos por eixo (por se tratar de cubo precisa de + 1 para o ofsset das arestas)
    int     get_BorderGrid() { return cellGrid + 2; /*resolution default = 17*/ }

    int   get_baseChunkSize() { return baseChunkSize0; }  // tamanho total do chunk em unidades de mundo  
    
    float   get_voxelSize(unsigned int lod) { return baseChunkSize0 * (1 << lod); /*return chunkSize / float(cellGrid);*/ } // equivalente a { return chunkSize / float(voxelGrid - 1); }   //real size of each voxel  // tamanho real de cada voxel
    //float   get_voxelSizeg(OctreeNode* n) { return n->edge_length() / get_voxelGrid(); /*return chunkSize / float(cellGrid);*/ } // equivalente a { return chunkSize / float(voxelGrid - 1); }   //real size of each voxel  // tamanho real de cada voxel
      
    float   lod_grid_size(int lod) const { return float(baseChunkSize0 << lod); }   // chunk_size * 2^lod

    int   get_MinChunkLod() { return minChunkLod; } 


private:
    int         cellGrid        = 16/*/2*/;           //  how many cells are in the grid                                           
    int         baseChunkSize0  = 16/*/2*/;           //  size of chunk in LOD 0        
    int         minChunkLod     = 2;    //4        //  min lod that a node can have to generate a chunk

    bool border = true;

    //constexpr SysVoxel() {}
    SysVoxel() = default;
    ~SysVoxel() {} // (opcional) destrutor privado
};


class SysOctree {
public:
    glm::vec3   camera = { 0, 0, 0 };
    const float BASE_CELL_SIZE = 1.0f;      // menor célula possível            octreeScale
    int         shellSize = 2;              // tamanho dos shells 
    int         maxDepthLod = 0;

    bool        automaticUpdate = true;
    float       automaticUpdateDistance = 64;
    float       autoMeshCoolDown; 

    //for keep default singleton
    static SysOctree& getInstance() {       //getVoxelGrid
        static SysOctree instance; // criado na primeira chamada
        return instance;
    }
    SysOctree(const SysOctree&) = delete;
    SysOctree& operator=(const SysOctree&) = delete;
     
    inline void set_camera(const glm::vec3& pos) { camera = pos; }

    inline float   get_baseChunkSize2() { return BASE_CELL_SIZE; }  // tamanho total do chunk em unidades de mundo  

     
    inline float lod_to_grid_size(const int lod) const {
        return (1 << (long)(lod + 1)) * BASE_CELL_SIZE; // should be times octreescale?
    } 

    inline glm::vec3 snap_to_grid(const glm::vec3& p /*pos*/, float g /*grid_size*/) const {
        return glm::floor(p / g) * g;
    }

    inline bool is_in_lod_shell(int lod, glm::vec3 pos, glm::vec3 cam_pos) const {
        float grid_size = lod_to_grid_size(lod) * 2.0;
        glm::vec3 lod_cam_pos = snap_to_grid(cam_pos, grid_size);
        glm::vec3 delta = abs(pos - lod_cam_pos);
        float dist = glm::max(delta.x, glm::max(delta.y, delta.z));
        return dist < (grid_size * shellSize);
    }

    // ----- LOD Query ----- 
    inline int lod_at(const glm::vec3& worldPos) const {   
        glm::vec3 pos = worldPos / BASE_CELL_SIZE;
        glm::vec3 cam = camera / BASE_CELL_SIZE;

        //NEW: use logarithm + adjustment.
        glm::vec3 delta = glm::abs(pos - cam) / (2.0f * shellSize);
        float maxDiff_chebyshev = glm::max(1.0f, glm::max(delta.x, glm::max(delta.y, delta.z)));
        int lod = glm::max(0, int(floor(glm::log2(maxDiff_chebyshev))));

        //O código calculou um LOD aproximado baseado na distância chunkizada.
        //MAS ele PODE errar em “±1” por causa do snap de grade, abaixo lidamos com isso.

        //Approximation is at most 1 off, so check if it should be +1 or -1.
        if (!is_in_lod_shell(lod, pos, cam)) return lod + 1;
        if (lod <= 0 || !is_in_lod_shell(lod - 1, pos, cam)) return lod;
        return lod - 1;
    }

    float lod_grid_size(int lod) { return (1 << (lod + sysv.get_MinChunkLod())) * BASE_CELL_SIZE; }


private:     
    //constexpr SysVoxel() {}
    SysOctree() = default;
    ~SysOctree() {} // (opcional) destrutor privado
};



/*

#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>

class LODController {
public:
    glm::vec3 camera = { 0,0,0 };
    int lodCount = 4;           // ex: LOD0..LOD5
    int shellSize = 2;          // tamanho dos shells
    int baseChunkSize = 16;     // tamanho de voxel do LOD0

    //int lodCount = 4;           // ex: LOD0..LOD5

    static constexpr float CHUNK_TO_LOD_UNITS = 1.0f / 16.0f;

    inline void set_camera(const glm::vec3& pos) { camera = pos; }

    inline float lod_grid_size(int lod) const {
        return float(baseChunkSize << lod); // chunk_size * 2^lod
    }

    // ----- LOD Query -----
    inline int lod_at(const glm::vec3& worldPos) const {
        glm::vec3 pos = worldPos * CHUNK_TO_LOD_UNITS;
        glm::vec3 cam = camera * CHUNK_TO_LOD_UNITS;

        for (int lod = 0; lod < lodCount; lod++)
            if (is_in_shell(lod, pos, cam))
                return lod;

        return lodCount - 1;
    }

private:
    inline glm::vec3 snap(const glm::vec3& p, float g) const {
        return glm::floor(p / g) * g;
    }

    bool is_in_shell(int lod, const glm::vec3& pos, const glm::vec3& cam) const {
        float grid = lod_grid_size(lod) * 2.0f;
        glm::vec3 camSnapped = snap(cam, grid);

        glm::vec3 delta = glm::abs(pos - camSnapped);
        float dist = glm::compMax(delta);

        return dist < (grid * shellSize);  //verificar pq * 2
    }
};

*/
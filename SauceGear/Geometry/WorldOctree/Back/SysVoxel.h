#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include "../../World/SurfaceNets/SurfaceNets.h"

#define sysv SysVoxel::getInstance()
#define syso SysVoxel::getInstance().octreeSys

struct SysVoxel {
    float isoLevel = 0.0f;         // isoLevel - nível de isosuperfície 
    glm::vec3 numChunksPerAxis = glm::vec3(5.0f, 2.0f, 5.0f); // quantos chunks criar em cada eixo
    OctreeSys octreeSys;


    void    set_cellGrid(int s) { cellGrid = s; }     //set_voxelGrid(int s) { cellGrid  = s; }
    void    set_base0ChunkSize(int s) { chunkSizeBase_0 = s; }

    int     get_cellGrid() { return cellGrid; }  // número de células  //ex : 16 
    // nº de pontos por eixo (por se tratar de cubo precisa de + 1 para o ofsset das arestas)
    int     get_voxelGrid() { return cellGrid + 1; /*resolution default = 17*/ }
    float   get_baseChunkSize() { return chunkSizeBase_0; }  // tamanho total do chunk em unidades de mundo 

    float   get_voxelSize(unsigned int lod) { return chunkSizeBase_0 * (1 << lod); /*return chunkSize / float(cellGrid);*/ } // equivalente a { return chunkSize / float(voxelGrid - 1); }   //real size of each voxel  // tamanho real de cada voxel

    //for keep default singleton
    static SysVoxel& getInstance() {       //getVoxelGrid
        static SysVoxel instance; // criado na primeira chamada
        return instance;
    }
    SysVoxel(const SysVoxel&) = delete;
    SysVoxel& operator=(const SysVoxel&) = delete;


private:
    int   cellGrid = 16;    //32 //64     //rsltCellsPerAxis  // número de voxels por eixo (_rsltPerAxis)  //int width_X = 32, height_Y = 32, dept_Z = 32; // cells count 
    float chunkSizeBase_0 = 16;           // tamanho físico de cada voxel = _WrdBdSize / _rsltPerAxis    

    constexpr SysVoxel() {}
    ~SysVoxel() {} // (opcional) destrutor privado
};

class OctreeSys {
public:
    glm::vec3   camera = { 0,0,0 };
    float       octreeScale = 1.0f;
    int         shellSize = 2;          // tamanho dos shells
    int         baseChunkSize = 16;     // tamanho de voxel do LOD0   
    int         maxDepthLod = 0;
    int         minChunkLod = 4;

    bool        automaticUpdate = true;
    float       automaticUpdateDistance = 64;
    float       autoMeshCoolDown;


    inline void set_camera(const glm::vec3& pos) { camera = pos; }

    inline float lod_grid_size(int lod) const {
        return float(baseChunkSize << lod); // chunk_size * 2^lod
    }
     
    // ----- LOD Query ----- 
    inline int lod_at(const glm::vec3& worldPos) const {
        float tsf2coordChunkLod = sysv.get_baseChunkSize();
        glm::vec3 pos = worldPos * tsf2coordChunkLod;
        glm::vec3 cam = camera * tsf2coordChunkLod;

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
     
private:  
    inline float lod_to_grid_size(const int lod) const {
        return (1 << (long)(lod + 1)) * octreeScale; // should be times octreescale?
    } 
    inline glm::vec3 snap_to_grid(const glm::vec3& p, /*grid_size*/ float g) const { return glm::floor(p / g) * g; }
    inline bool is_in_lod_shell(int lod, glm::vec3 pos, glm::vec3 cam_pos) const {
        float grid_size = lod_to_grid_size(lod) * 2.0;
        glm::vec3 lod_cam_pos = snap_to_grid(cam_pos, grid_size);
        glm::vec3 delta = abs(pos - lod_cam_pos);
        float dist = glm::max(delta.x, glm::max(delta.y, delta.z));
        return dist < (grid_size * shellSize);
    }
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
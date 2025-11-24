#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include "../../World/SurfaceNets/SurfaceNets.h"

class OctreeSys {
public:
    glm::vec3 camera = { 0,0,0 };
    //int lodCount = 4;           // ex: LOD0..LOD5

    float octreeScale = 1.0f;
    int _shellSize = 2;          // tamanho dos shells
    int baseChunkSize = 16;     // tamanho de voxel do LOD0  
     
    int maxDepthLod = 0;  
    int minChunkLod = 4;

    bool _automaticUpdate = true;   float _automaticUpdateDistance = 64;
    float _autoMeshCoolDown;


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
        glm::vec3 delta = glm::abs(pos - cam) / (2.0f * _shellSize);
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
        return dist < (grid_size * _shellSize);
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
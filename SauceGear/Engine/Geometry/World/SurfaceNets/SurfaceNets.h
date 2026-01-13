#pragma once 
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <limits>
#include <iostream> 
#include <memory>
#include <unordered_set>  

// forward
struct MeshAsset;

static constexpr uint32_t NULL_VERTEX = std::numeric_limits<uint32_t>::max();

// Hash para glm::ivec3
struct IVec3Hash {
    std::size_t operator()(const glm::ivec3& v) const noexcept {
        std::size_t h1 = std::hash<int>{}(v.x);
        std::size_t h2 = std::hash<int>{}(v.y);
        std::size_t h3 = std::hash<int>{}(v.z);
        // combina os hashes
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};
struct Vec3Hash {
    std::size_t operator()(const glm::vec3& v) const noexcept {
        std::size_t h1 = std::hash<float>{}(v.x);
        std::size_t h2 = std::hash<float>{}(v.y);
        std::size_t h3 = std::hash<float>{}(v.z);
        // Combina os hashes
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

struct Vec3Equal {
    bool operator()(const glm::vec3& a, const glm::vec3& b) const {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};  


struct ChunkBuffer { //Chunk
    std::vector<float>      densityMap;    //sdf

    //std::vector<glm::vec4>  positions;                                                            
    //std::vector<glm::vec4>  normals;     // not normalized (normalize on GPU if desired)         
    //std::vector<uint32_t>   indices;

    // bookkeeping for quads generation
    std::vector<glm::vec3>  surface_points;
    std::vector<uint32_t>   surface_strides;
    std::vector<uint32_t>   stride_to_index;

    // debug: points at cube corners for cubes that cross the surface 
    std::unordered_set<glm::vec3, Vec3Hash, Vec3Equal> debug_corners;       //std::unordered_set<glm::ivec3, IVec3Hash> debug_corners;  //std::unordered_set<glm::ivec3> debug_corners;

    void reset(size_t array_size) {
        //positions.clear(); //normals.clear(); //indices.clear();
        surface_points.clear();
        surface_strides.clear();
        stride_to_index.assign(array_size, NULL_VERTEX);

        densityMap.assign(array_size, 1e6f);
    }
};

struct Chunk {
    std::shared_ptr<MeshAsset>      mesh;
    std::unique_ptr<ChunkBuffer>    buff;     //sdf

    glm::vec3 coord; glm::ivec3     coordWorld;  
    int                             lod = 0;

    float                           dbg = 0;
    Entity                          entity = INVALID_ENTITY;

    Chunk(size_t d) {
        buff = std::make_unique<ChunkBuffer>();  
        resizeDensityMap(d);
    }

    //Note that consider that d is size final total
    void resizeDensityMap(size_t dim){ // (Re)aloca o vetor se o tamanho mudou
        size_t total = size_t(dim) * dim * dim; 
        if (buff->densityMap.size() != total) buff->densityMap.resize(total, 1);
        currentVoxelGrid = total;
    }

    int idx(int x, int y, int z) const { 
        uint32_t dim = currentVoxelGrid;
        return (z * dim + y) * dim + x;         // (z * sy + y) * sx + x; 
    }            
    float get(int x, int y, int z) const       { return buff->densityMap[idx(x, y, z)]; }
    void  set(int x, int y, int z, float v)    { buff->densityMap[idx(x, y, z)] = v; }

private:
    uint32_t currentVoxelGrid;  //Dim
};  







//namespace SurfaceNetsGPU {
//    // callback when mesh ready (Mesh* may be created by engine)
//    Mesh* Generate(const Chunk& sdf, glm::vec3 uOffset, GLuint computeProgram, GLuint ssboSDF = 0);
//}

/*
/// CPU implementation: produces Mesh (vertices, normals, uvs, indices)
namespace SurfaceNetsCPU {
    // callback when mesh ready (Mesh* may be created by engine)
    Mesh* Generate(const Chunk& sdf, SurfaceNetsBuffer& out);
}
*/

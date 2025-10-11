#pragma once 
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <limits>
#include <iostream>
#include "../NoiseGenerator/SphereSDF.h" 
#include <memory>
#include <unordered_set> 

static constexpr uint32_t NULL_VERTEX = std::numeric_limits<uint32_t>::max();

// forward
struct Mesh; 

struct SurfaceNetsParams {
    //int cellDimension = 32;    //int width_X = 32, height_Y = 32, dept_Z = 32; // cells count 

    int   cellDimension = 32;        //rsltCellsPerAxis        // número de voxels por eixo (_rsltPerAxis)
    float worldSize = 32;        // tamanho físico de cada voxel = _WrdBdSize / _rsltPerAxis 

    float voxelSize = 1.0f;
    float isoLevel = 0.0f;         // isoLevel - nível de isosuperfície

    glm::vec3 globalOffset = glm::vec3(0);  // posição base do mundo
    glm::vec3 numChunks    = glm::vec3(4); // quantos chunks criar em cada eixo
     
    SurfaceNetsParams(int cellDimension = 32, float worldSize = 1.0f, float isoLevel = 0.0f)
        : cellDimension(cellDimension), worldSize(worldSize), isoLevel(isoLevel) 
    {
        updtVoxelSize();
    }; 

    void setWorldSize(int w)        { worldSize = w; updtVoxelSize(); }
    void setCellDimension(int c)    { cellDimension = c + 1;  updtVoxelSize();}
    void updtVoxelSize()            { voxelSize = worldSize / float(cellDimension); } 
}; 


struct VoxelGrid {
    int sx, sy, sz;
    std::vector<float> density;

    VoxelGrid(int sx = 0, int sy = 0, int sz = 0) : sx(sx), sy(sy), sz(sz), density(sx* sy* sz, 1.0f) {}

    inline int idx(int x, int y, int z) const { return (z * sy + y) * sx + x; }
    float get(int x, int y, int z) const { return density[idx(x, y, z)]; }
    void set(int x, int y, int z, float v) { density[idx(x, y, z)] = v; }
};


//struct VoxelGrid {
//    // contiguous storage: dimX+1 * dimY+1 * dimZ+1 densities
//    int sx, sy, sz; // points count (cells+1)
//    std::vector<float> density; // size sx*sy*sz 
//
//    VoxelGrid(int sx = 0, int sy = 0, int sz = 0) : sx(sx), sy(sy), sz(sz), density(sx* sy* sz, 1.0f) {
//        const int dim = 32 + 0;
//        const float radius = 10.0f;
//        density = GeneratorMap::GenerateSphereSDF(dim, radius);
//    }
//
//    inline int get_LinearizeId(int x, int y, int z) const { return density[(z * sy + y) * sx + x]; }
//    inline int get_LinearizeId(glm::vec3 d) const { return density[(d.z * sy + d.y) * sx + d.x]; }
//
//    inline int idx(int x, int y, int z) const { return (z * sy + y) * sx + x; }
//    float get(int x, int y, int z) const { return density[idx(x, y, z)]; }
//    void set(int x, int y, int z, float v) { density[idx(x, y, z)] = v; }
//};

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


struct SurfaceNetsBuffer { //Chunk  
    //VoxelGrid* grid;
    //std::unique_ptr<VoxelGrid> grid;

    std::vector<glm::vec4> positions;                                                           //std::vector<glm::vec3> positions;
    std::vector<glm::vec4> normals;     // not normalized (normalize on GPU if desired)         //std::vector<glm::vec3> normals;
    std::vector<uint32_t> indices;

    // bookkeeping for quads generation
    std::vector<glm::vec3> surface_points;
    std::vector<uint32_t> surface_strides;
    std::vector<uint32_t> stride_to_index;


    // debug: points at cube corners for cubes that cross the surface
    //std::unordered_set<glm::ivec3, IVec3Hash> debug_corners;
    std::unordered_set<glm::vec3, Vec3Hash, Vec3Equal> debug_corners;
    //std::unordered_set<glm::ivec3> debug_corners;

    void reset(size_t array_size) {
        positions.clear();
        normals.clear();
        indices.clear();
        surface_points.clear();
        surface_strides.clear();
        stride_to_index.assign(array_size, NULL_VERTEX);
    }

    SurfaceNetsBuffer() {
        //grid = std::make_unique<VoxelGrid>(32 + 0, 32 + 0, 32 + 0);
    }
};


/// CPU implementation: produces Mesh (vertices, normals, uvs, indices)
namespace SurfaceNetsCPU {
    // callback when mesh ready (Mesh* may be created by engine)
    Mesh* Generate(const VoxelGrid& sdf, const SurfaceNetsParams& params, SurfaceNetsBuffer& out);
}

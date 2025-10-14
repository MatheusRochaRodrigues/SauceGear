#pragma once 
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <limits>
#include <iostream>
#include "../NoiseGenerator/SphereSDF.h" 
#include <memory>
#include <unordered_set> 

#define sysv SysVoxel::getInstance()

// forward
struct Mesh;

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

struct SysVoxel {
    float isoLevel = 0.0f;         // isoLevel - nível de isosuperfície 
    glm::vec3 numChunksPerAxis = glm::vec3(2); // quantos chunks criar em cada eixo

    void    set_cellGrid(int s)  { cellGrid  = s; }     //set_voxelGrid(int s) { cellGrid  = s; }
    void    set_chunkSize(int s) { chunkSize = s; }

    int     get_cellGrid()   { return cellGrid; }  // número de células
    int     get_voxelGrid()  { return cellGrid + 1; }  // nº de pontos por eixo (por se tratar de cubo precisa de + 1 para o ofsset das arestas)
    float   get_chunkSize()  { return chunkSize; }  // tamanho total do chunk em unidades de mundo
    float   get_voxelSize()  { return chunkSize / float(cellGrid); } // equivalente a { return chunkSize / float(voxelGrid - 1); }   //real size of each voxel  // tamanho real de cada voxel
       
    //for keep default singleton
    static SysVoxel& getInstance() {       //getVoxelGrid
        static SysVoxel instance; // criado na primeira chamada
        return instance;
    } 
    SysVoxel(const SysVoxel&) = delete;
    SysVoxel& operator=(const SysVoxel&) = delete;

private:
    int   cellGrid = 32;    //rsltCellsPerAxis  // número de voxels por eixo (_rsltPerAxis)  //int width_X = 32, height_Y = 32, dept_Z = 32; // cells count 
    float chunkSize = 32;        // tamanho físico de cada voxel = _WrdBdSize / _rsltPerAxis    

    constexpr SysVoxel() {}
    ~SysVoxel() {} // (opcional) destrutor privado
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
        //positions.clear();
        //normals.clear();
        //indices.clear();
        surface_points.clear();
        surface_strides.clear();
        stride_to_index.assign(array_size, NULL_VERTEX);
    }
};

struct Chunk {
    std::unique_ptr<Mesh>           mesh;
    std::unique_ptr<ChunkBuffer>    buff;     //sdf

    glm::vec3 coord; glm::ivec3     coordWorld;  
    int                             lod = 0;

    Chunk() { 
        buff = std::make_unique<ChunkBuffer>();  
        resizeDensityMap();
    }

    void resizeDensityMap(){ // (Re)aloca o vetor se o tamanho mudou
        uint32_t dim = SysVoxel::getInstance().get_voxelGrid(); 
        size_t total = size_t(dim) * dim * dim;
        if (buff->densityMap.size() != total) buff->densityMap.resize(total, 1);
    }

    int idx(int x, int y, int z) const { 
        uint32_t dim = SysVoxel::getInstance().get_voxelGrid();
        return (z * dim + y) * dim + x;         // (z * sy + y) * sx + x; 
    }            
    float get(int x, int y, int z) const       { return buff->densityMap[idx(x, y, z)]; }
    void  set(int x, int y, int z, float v)    { buff->densityMap[idx(x, y, z)] = v; }
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

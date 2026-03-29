#pragma once  
#include <iostream>
#include <glm/glm.hpp>
#include "../../Geometry/Voxel/Octree/DCNode.h" 
#include "../../Geometry/Voxel/Octree/OctreeBuilder.h"
#include "../../Geometry/Voxel/DC/DCMeshBuilder.h"
#include "../../Graphics/Vertex.h"
#include "../WorldController.h" 

// ============================================================
// CHUNK
// ============================================================

enum class ChunkState : uint8_t
{
    Empty,
    NeedsBuild,
    Building,
    MeshReady,
    Stitched,
    Ready
};


struct alignas(64) Chunk
{
    ivec3                   coord;
    int                     lod;
    std::atomic<ChunkState> state = ChunkState::Empty;
    std::unique_ptr<DCNode> root;
    VertexBuffer            vertexBuffer;
    IndexBuffer             indexBuffer;
     
    std::atomic<bool>       pendingRemoval = false;
    uint32_t                lastVisitedFrame = 0;

    // Calcula posição em mundo 
    int ChunkWorldSize() const {
        return CHUNK_SIZE * BASE_CELL_SIZE * (1 << lod);
    }

    // BUILD FUNCTION      
    void BuildChunk(DensityCache& densityCache) {       //WorldChunkStorage& world
        try {
            std::cout << "makeChunk start for chunk " << coord.x << "," << coord.y << "," << coord.z << "\n";
            state = ChunkState::Building;

            //BuildSDF();  //BuildEdgeCache();

            BuildContext_CK ctx(lod, &densityCache);

            //float voxelSize = worldSize / resolution;
            ivec3 worldMin = ivec3(coord * ChunkWorldSize());

            root = std::unique_ptr<DCNode>(
                BuildOctree(worldMin, ChunkWorldSize(), ctx)
            );

            // Checagem de segurança
            if (!root) {
                std::cout << "Chunk at " << coord.x << "," << coord.y << "," << coord.z << " has no surface! Skipping mesh generation.\n";
                state = ChunkState::Empty;
                return;
            }

            GenerateMeshFromOctree(root.get(), vertexBuffer, indexBuffer);

            std::cout << "Mesh generated for chunk " << coord.x << "," << coord.y << "," << coord.z << "\n";
            state = ChunkState::Ready;
        }
        catch (const std::exception& e) {
            std::cerr << "Exception in makeChunk(): " << e.what() << "\n";
            state = ChunkState::Empty;
            root.reset();
        }
        catch (...) {
            std::cerr << "Unknown crash in makeChunk() for chunk "
                << coord.x << "," << coord.y << "," << coord.z << "\n";
            state = ChunkState::Empty;
            root.reset();
        }

    }

};













/*
void Build(WorldChunkStorage& world)
{
    state = ChunkState::Building;

    // Snapshot neighbors safely
    auto neighbors = GetSpatialNeighbors(world);

    // Aqui você integra:
    // - BuildOctree(...)
    // - Costura adaptativa usando neighbors
    // - GenerateMesh(...)

    state = ChunkState::Ready;
}

// =========================================
// LOD RELATIONS
// =========================================

ivec3 ParentCoord() const
{
    return ivec3(
        floor(float(coord.x) / 2.0f),
        floor(float(coord.y) / 2.0f),
        floor(float(coord.z) / 2.0f)
    );
}

std::vector<ivec3> ChildrenCoords() const
{
    ivec3 base = coord * 2;

    return {
        base + ivec3(0,0,0),
        base + ivec3(1,0,0),
        base + ivec3(0,1,0),
        base + ivec3(1,1,0),
        base + ivec3(0,0,1),
        base + ivec3(1,0,1),
        base + ivec3(0,1,1),
        base + ivec3(1,1,1)
    };
}

// =========================================
// NEIGHBOR LOOKUP
// =========================================

std::vector<std::shared_ptr<Chunk>>
GetSpatialNeighbors(const WorldChunkStorage& world)
{
    std::vector<std::shared_ptr<Chunk>> result;

    static const ivec3 dirs[6] =
    {
        { 1, 0, 0 }, { -1, 0, 0 },
        { 0, 1, 0 }, { 0,-1, 0 },
        { 0, 0, 1 }, { 0, 0,-1 }
    };

    // Same LOD
    for (auto& d : dirs)
    {
        if (auto n = world.GetChunk(coord + d, lod))
            result.push_back(n);
    }

    // Parent LOD
    if (lod + 1 < CLIP_LEVELS)
    {
        if (auto p = world.GetChunk(ParentCoord(), lod + 1))
            result.push_back(p);
    }

    // Children LOD
    if (lod > 0)
    {
        for (auto& c : ChildrenCoords())
        {
            if (auto child = world.GetChunk(c, lod - 1))
                result.push_back(child);
        }
    }

    return result;
}

*/
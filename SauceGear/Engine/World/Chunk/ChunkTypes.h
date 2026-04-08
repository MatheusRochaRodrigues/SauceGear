#pragma once
#include <glm/glm.hpp>
using namespace glm;

// ChunkTypes / ChunkDef / Chunk Helper

// CHUNK KEY  
struct ChunkKey
{
    ivec3 coord;
    int lod;

    bool operator==(const ChunkKey& o) const
    {
        return coord == o.coord && lod == o.lod;
    }
};

struct ChunkKeyHasher
{
    size_t operator()(const ChunkKey& k) const
    {
        size_t h = std::hash<int>()(k.coord.x);
        h ^= std::hash<int>()(k.coord.y) << 1;
        h ^= std::hash<int>()(k.coord.z) << 2;
        h ^= std::hash<int>()(k.lod) << 3;
        return h;
    }
};

// CHUNK REQUEST (priority queue)  
struct ChunkRequest
{
    ChunkKey key;
    float priority;

    bool operator<(const ChunkRequest& other) const
    {
        return priority > other.priority; // menor = mais importante
    }
};



//---------------------------------------------------------------------------
// EXTRAS
//---------------------------------------------------------------------------


//  Thread
//thread_local ThreadArena<DCNode> arena;
//  Garbage
//uint32_t                lastVisitedFrame = 0; 













// LEGACY



/*
// BUILD FUNCTION
void BuildChunk(DensityCache& densityCache, ChunkStreamingBridge* bridge) {       //WorldChunkStorage& world
    try {
        std::cout << "makeChunk start for chunk " << coord.x << "," << coord.y << "," << coord.z << "\n";
        state = ChunkState::Building;

        //BuildSDF();  //BuildEdgeCache();

        int chunkWorldSize = DataWorld::ChunkWorldSize(lod);
        ivec3 worldMin = coord * chunkWorldSize;                //float voxelSize = worldSize / resolution;

        BuildContext_CK ctx(worldMin, lod, &densityCache);

        //SYNC
        root = std::unique_ptr<DCNode>(
            BuildOctree(worldMin, chunkWorldSize, ctx)
        );

        //ASYNC
        BuildOctreeParallel(
            worldMin, chunkWorldSize, ctx,
            [this](DCNode* node)
            {
                submitJobChunk(node);
            }
        );

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

void submitJobChunk(DCNode* rootOctree) {
    // Checagem de segurança
    if (!rootOctree) {
        std::cout << "Chunk at " << coord.x << "," << coord.y << "," << coord.z << " has no surface! Skipping mesh generation.\n";
        state = ChunkState::Empty;
        return;
    }

    root.reset(rootOctree);

    GenerateMeshFromOctree(root.get(), vertexBuffer, indexBuffer);

    std::cout << "Mesh generated for chunk " << coord.x << "," << coord.y << "," << coord.z << "\n";
    state = ChunkState::Ready;

    bridge->readyChunks.push(chunk); // envia para ECS
    pending.erase(key);
}
*/


/*

            //  Async
            DCNode* rawRoot = new DCNode;
            rawRoot->min = worldMin;
            rawRoot->size = chunkWorldSize;
            rawRoot->type = Node_Internal;

            root.reset(rawRoot);

            // CALLBACK FINAL
            auto* counter = jobSystem.CreateCounter( 1, [this]()
            {
                OnOctreeBuilt();
            });









    auto* counter = jobSystem.CreateCounter(1, [chunk]()
    {
        chunk->state = ChunkState::Ready;
        bridge->readyChunks.push(chunk);
    });
*/







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
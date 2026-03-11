#pragma once 
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <glm/glm.hpp>

#include "../Voxel/Octree/DCNode.h"
#include "../Voxel/Octree/OctreeBuilder.h"
#include "../Voxel/DC/DCMeshBuilder.h"
#include "ThreadWorker.h"

using namespace glm;

// ============================================================
// CONFIG
// ============================================================

constexpr int   CHUNK_SIZE = 32;            //Grid
constexpr int   BASE_CELL_SIZE = 1.0;      //float BASE_CELL_SIZE = 1.0f;
constexpr int   CLIP_LEVELS = 3;    //5
constexpr int   BASE_RING_RADIUS = 2;

// FORWARD DECLARATION  
struct Chunk;

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

// ============================================================
// WORLD STORAGE (GLOBAL)
// ============================================================

class WorldChunkStorage {
public:
    std::unordered_map< ChunkKey, std::shared_ptr<Chunk>, ChunkKeyHasher > chunks;
    mutable std::shared_mutex mutex;

    std::shared_ptr<Chunk> GetChunk(const ivec3& coord, int lod) const {
        std::shared_lock lock(mutex);

        auto it = chunks.find({ coord, lod });
        if (it != chunks.end()) return it->second;

        return nullptr;
    }

    void Insert(const ChunkKey& key, std::shared_ptr<Chunk> chunk) {
        std::unique_lock lock(mutex);
        chunks[key] = chunk;
    }

    void Remove(const ChunkKey& key) {
        std::unique_lock lock(mutex);
        chunks.erase(key);
    }
};

// ============================================================
// CHUNK
// ============================================================

enum class ChunkState : uint8_t
{
    Empty,
    NeedsBuild,
    Building,
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
    
        // Calcula posição em mundo 
    int ChunkWorldSize() const {
        return CHUNK_SIZE * BASE_CELL_SIZE * (1 << lod);
    } 

    // =========================================
    // BUILD FUNCTION 
    // =========================================    

    void BuildChunk(WorldChunkStorage& world) { 
        try {
            std::cout << "makeChunk start for chunk " << coord.x << "," << coord.y << "," << coord.z << "\n";
            state = ChunkState::Building;

            root = std::unique_ptr<DCNode>(
                BuildOctree( ivec3(coord * ChunkWorldSize()), CHUNK_SIZE )
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
    */

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

};

// ============================================================
// CLIPMAP SYSTEM
// ============================================================

// CLIP LEVEL  
struct ClipLevel
{
    int lod;
    int ringRadius;
};

class ClipmapSystem
{
public:

    WorldChunkStorage world;
    ClipLevel levels[CLIP_LEVELS];

    void Initialize()
    {
        for (int i = 0; i < CLIP_LEVELS; i++)
        {
            levels[i].lod = i;
            //levels[i].ringRadius = BASE_RING_RADIUS << i;
            levels[i].ringRadius = BASE_RING_RADIUS;
        }
    }

    float ChunkWorldSize(int lod) const {
        return CHUNK_SIZE * BASE_CELL_SIZE * float(1 << lod);
    }

    void Update(const vec3& cameraPos)
    {
        for (int i = 0; i < CLIP_LEVELS; i++)
            UpdateLevel(levels[i], cameraPos);
    }

private:

    void UpdateLevel(ClipLevel& level, const vec3& cameraPos) {
        // Convert world-space camera position into clip-level grid coordinates. (We divide by the current LOD's world size (chunk_size * 2^lod))
        // to find which chunk cell the camera is inside at this clip level.
        ivec3 center = ivec3(
            floor(cameraPos / ChunkWorldSize(level.lod))
        );

        std::unordered_set<ChunkKey, ChunkKeyHasher> required;

        int r = level.ringRadius;

        for (int z = -r; z <= r; z++)
            for (int y = -r; y <= r; y++)
                for (int x = -r; x <= r; x++)
                {
                    // Exclude inner region (true ring)
                    if (level.lod > 0)
                    {
                        int inner = BASE_RING_RADIUS << (level.lod - 1);

                        if (abs(x) <= inner &&
                            abs(y) <= inner &&
                            abs(z) <= inner)
                            continue;
                    }

                    ivec3 coord = center + ivec3(x, y, z);

                    ChunkKey key{ coord, level.lod };
                    required.insert(key);

                    if (!world.GetChunk(coord, level.lod)) {
                        CreateChunk(key);
                    }

                }

        RemoveUnused(required, level.lod);
    }

    void CreateChunk(const ChunkKey& key)
    {
        auto chunk = std::make_shared<Chunk>();
        chunk->coord = key.coord;
        chunk->lod = key.lod;
        chunk->state = ChunkState::NeedsBuild;

        world.Insert(key, chunk);

        // Aqui você pode usar seu ThreadPool 
        //gThreadPool.Enqueue([chunk, this]() { chunk->BuildChunk(world); }); 
        chunk->BuildChunk(world);
    }

    void RemoveUnused(const std::unordered_set<ChunkKey, ChunkKeyHasher>& required, int lod)
    {
        std::vector<ChunkKey> toRemove;

        {
            std::shared_lock lock(world.mutex);

            for (auto& [key, chunk] : world.chunks)
            {
                if (key.lod != lod)
                    continue;

                if (required.find(key) == required.end())
                    toRemove.push_back(key);
            }
        }

        for (auto& k : toRemove)
            world.Remove(k);
    }

};



/*
blockCoord = floor(worldPos / blockSize)
block = cache.get(blockCoord)

local = worldPos % blockSize

return block->sdf[local]




SampleSDF(worldPos)
*/
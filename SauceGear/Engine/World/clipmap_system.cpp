#pragma once 
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <glm/glm.hpp> 

#include "ThreadWorker.h"  
#include "../Utils/Threads/JobSystem.h"  
#include "ChunkStreamingBridge.h" 
#include "WorldController.h"
#include "../Geometry/Voxel/Density/DensityBrickCache.h"
#include "Chunk/Chunk.h" 

using namespace glm;   

// ============================================================
// WORLD STORAGE (GLOBAL)
// ============================================================

class WorldChunkStorage {
public:
    std::unordered_map< ChunkKey, std::shared_ptr<Chunk>, ChunkKeyHasher > chunks;
    //  std::vector<ChunkKey> required; required.reserve(512);

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

    void Remove(const ChunkKey& key, ChunkStreamingBridge* bridge) {
        std::unique_lock lock(mutex); 

        //ECS  
        auto it = chunks.find(key);
        if (it != chunks.end()) {
            auto chunk = it->second;

            if (!chunk->pendingRemoval.exchange(true))
                bridge->removeChunks.push(chunk);
        }

        //System World
        chunks.erase(key);
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
    ivec3 lastCenter;
};

class ClipmapSystem
{
public:
    // STRUCTS
    WorldChunkStorage world;

    DensityCache densityCache;

    ClipLevel levels[CLIP_LEVELS]; 

    // CONFIG  
    float directionBias = 2.0f;

    int chunksPerFrame = THREAD_COUNT;   // budget      4

    // STREAMING            
    Bridge bridge;

    //JobSystem jobSystem;
     
    void Initialize()
    {
        for (int i = 0; i < CLIP_LEVELS; i++)
        {
            levels[i].lod = i; 
            levels[i].ringRadius = BASE_RING_RADIUS;        //levels[i].ringRadius = BASE_RING_RADIUS << i;
            levels[i].lastCenter = ivec3(FLT_MAX);
        }

        //jobSystem.Init();
    } 

    void Update(const vec3& cameraPos, const vec3& cameraForward)
    {
        frameID++;

        for (int i = 0; i < CLIP_LEVELS; i++)
            UpdateLevel(levels[i], cameraPos, cameraForward);
    }

private:

    void UpdateLevel(ClipLevel& level, const vec3& cameraPos, const vec3& cameraForward)  {
        if (bridge.ECSBridge == nullptr) std::cout << "ECS Bridge is NULL" << std::endl;

        float worldSize = DataWorld::ChunkWorldSize(level.lod);

        // Convert world-space camera position into clip-level grid coordinates. (We divide by the current LOD's world size (chunk_size * 2^lod)), to find which chunk cell the camera is inside at this clip level.
        ivec3 center = ivec3(floor(cameraPos / worldSize)); 

        bool moved = (center != level.lastCenter); 
        // avoid recalculating if nothing change
        if (moved) {
            level.lastCenter = center;

            int r = level.ringRadius;

            std::unordered_set<ChunkKey, ChunkKeyHasher> required;

            for (int z = -r; z <= r; z++)
                for (int y = -r; y <= r; y++)
                    for (int x = -r; x <= r; x++)
                    {
                        // remove inner ring
                        if (level.lod > 0)
                        {
                            int inner = BASE_RING_RADIUS / 2;

                            /*int outer = level.ringRadius;
                            int inner = (level.lod == 0) ? 0 : levels[level.lod - 1].ringRadius;*/

                            if (abs(x) <= inner &&
                                abs(y) <= inner &&
                                abs(z) <= inner)
                                continue;
                        }

                        ivec3 coord = center + ivec3(x, y, z);

                        ChunkKey key{ coord, level.lod };
                        required.insert(key);

                        // is already exists?
                        auto chunk = world.GetChunk(coord, level.lod);
                        if (chunk) {     //if (chunk->state != ChunkState::Empty) continue;
                            if (!chunk->pendingRemoval) chunk->pendingRemoval.exchange(false);
                            continue;                   //chunk->lastVisitedFrame = frameID;
                        }


                        if (bridge.pending.insert(key).second)
                        {
                            // PRIORITY 
                            vec3 delta = vec3(coord - center);
                            float dist = length(delta);

                            vec3 dir = normalize(delta + vec3(0.0001f));
                            float alignment = dot(dir, cameraForward);

                            //float priority = dist - alignment * directionBias;
                            float priority = dist * (1.0f - std::max(0.0f, alignment) * 0.5f);

                            // REQUEST
                            bridge.requestQueue.push({ key, priority });
                        }

                        //std::cout << "Chunk built: " << coord.x << "," << coord.y << "," << coord.z << "\n";

                        //Você pode expandir o grid na direção da câmera:   Você pode expandir o grid na direção da câmera:
                        //ivec3 bias = ivec3(round(cameraForward * 2.0f));  //coord = center + ivec3(x, y, z) + bias;
                    } 
            
            RemoveUnused(required, level.lod);

        }

        ProcessChunkRequests();

    }

    // PROCESS REQUESTS (BUDGET CONTROL)  
    void ProcessChunkRequests()
    {
        int budget = chunksPerFrame;

        while (budget-- > 0 && !bridge.requestQueue.empty())
        {
            ChunkRequest req = bridge.requestQueue.top();
            bridge.requestQueue.pop();

            CreateChunk(req.key);
        }
    }

    //With Required
    void RemoveUnused(const std::unordered_set<ChunkKey, ChunkKeyHasher>& required, int lod)
    {
        std::vector<ChunkKey> toRemove;

        {
            std::shared_lock lock(world.mutex);

            for (auto& [key, chunk] : world.chunks)
            {
                if (key.lod != lod)
                    continue;

                /*if (chunk->state == ChunkState::Building) continue;*/     //RESOLVER AQ

                if (required.find(key) == required.end())
                    toRemove.push_back(key);
            }
        }

        for (auto& k : toRemove) {
            /*
            std::shared_ptr<Chunk> chunk;

            {
                std::shared_lock lock(world.mutex);
                auto it = world.chunks.find(k);
                if (it != world.chunks.end())
                    chunk = it->second;
            }

            if (chunk) bridge->removeChunks.push(chunk);
            */

            //Remove from World
            world.Remove(k, bridge.ECSBridge);        // world.Remove(k);
        }
    }  

    // CREATE CHUNK (ASYNC)  
    void CreateChunk(const ChunkKey& key)
    {
        auto chunk = std::make_shared<Chunk>();

        chunk->coord = key.coord;
        chunk->lod = key.lod;
        chunk->state = ChunkState::NeedsBuild;

        world.Insert(key, chunk);

        // JOB SYSTEM
        BuildChunk(chunk, densityCache, &bridge); 

        // MULTITHREAD
        /*
        gThreadPool.Enqueue([chunk, this, key]()
        {
            chunk->BuildChunk(densityCache);     //(world, densityCache)

            // após mesh pronta
            //chunk->state = ChunkState::MeshReady;

            if (chunk->state == ChunkState::Ready) 
                bridge->readyChunks.push(chunk); // envia para ECS 

            pending.erase(key);
        });
        */

        //SEQUENTIAL
        /*
        chunk->BuildChunk(densityCache);
        if (chunk->state == ChunkState::Ready)
            readyChunks->push(chunk); // envia para ECS 
        */
    } 


//========================================================
//  STREAMING LOGIC  
//========================================================
private:

    // STREAMING LOGIC  
    void UpdateStreaming(const vec3& cameraPos, const vec3& cameraForward)
    {
        for (int i = 0; i < CLIP_LEVELS; i++)
        {
            UpdateLevel(levels[i], cameraPos, cameraForward);
        }
    } 

    // STITCHING (HOOK) 

    //void ProcessStitching()
    //{
    //    for (auto& [key, chunk] : clipmap.world.chunks)
    //    {
    //        if (chunk->state == ChunkState::MeshReady)
    //        {
    //            TryStitchChunk(chunk);
    //        }
    //    }
    //}

    //void TryStitchChunk(std::shared_ptr<Chunk> chunk)
    //{
    //    auto neighbors = chunk->GetSpatialNeighbors(clipmap.world);

    //    for (auto& n : neighbors)
    //    {
    //        if (!n || n->state < ChunkState::MeshReady)
    //            return;
    //    }

    //    // aqui você pluga sua costura adaptativa
    //    // Stitch(chunk, neighbors);

    //    chunk->state = ChunkState::Ready;
    //}

private:
    uint32_t frameID = 1;
};









/*
//Without Required
void RemoveUnused(int lod)
{
    std::vector<ChunkKey> toRemove;

    {
        std::shared_lock lock(world.mutex);

        for (auto& [key, chunk] : world.chunks)
        {
            if (key.lod != lod)
                continue;

            if (chunk->lastVisitedFrame != frameID)
                toRemove.push_back(key);
        }
    }

    for (auto& k : toRemove)
    {
        auto chunk = world.GetChunk(k.coord, k.lod);

        if (chunk)
            bridge->removeChunks.push(chunk);

        world.Remove(k, bridge);    //world.Remove(k);
    }
}
*/




// ALINHAMENTO ENTRE LODS
//int align = 1 << level.lod; center = (center / align) * align;
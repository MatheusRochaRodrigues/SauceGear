#include "Chunk.h"  
#include "../WorldController.h" 
#include "../../Geometry/Voxel/dual_octree_mesh/Data/DCNode.h"   
#include "../../Geometry/Voxel/dual_octree_mesh/Async/Multi_Octree/Multi_OctreeBuilder.h"
#include "../../Geometry/Voxel/dual_octree_mesh/Async/Multi_DC/Kernel_DC.h" 
//#include "../../Geometry/Voxel/dual_octree_mesh/Async/Multi_DC/Multi_DCMeshBuilder.h" 
#include "../ChunkStreamingBridge.h"     

//-------------------------------------------------------------------
// Chunk
//-------------------------------------------------------------------

void submitJobChunk(std::shared_ptr<Chunk> c, DCNode* rootOctree, Bridge* bridge) {
    if (!c) {
        std::cout << "-------------------- CHUNK VAZADO ------------------ \n";
        return;
    }

    if (!rootOctree) {
        std::cout << "Chunk at " << c->coord.x << "," << c->coord.y << "," << c->coord.z << " has no surface! Skipping mesh generation.\n";
        c->state = ChunkState::Empty;
        return;
    }

    std::cout << "Mesh generated for chunk " << c->coord.x << "," << c->coord.y << "," << c->coord.z << "\n";
    MultiBuilder::GenerateMeshFromOctree_MultiThread(rootOctree, c->vertexBuffer, c->indexBuffer);
    std::cout << "Mesh generated for chunk " << c->coord.x << "," << c->coord.y << "," << c->coord.z << "\n";
    //return; 
    /*
    if (!c) {
        std::cout << "-------------------- CHUNK VAZADO ------------------ \n";
        return;
    }

    // Checagem de segurança
    if (!rootOctree) {
        std::cout << "Chunk at " << c->coord.x << "," << c->coord.y << "," << c->coord.z << " has no surface! Skipping mesh generation.\n";
        c->state = ChunkState::Empty;
        return;
    }

    //LEMBRESSE DE APAGAR O CK_CHUNKCONTEXT
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout << "   22222 \n";
    //rootOctree = Prune(rootOctree);

    if (rootOctree == nullptr) return;

    c->root.reset(rootOctree);

    std::cout << "   111111111 \n";

    GenerateMeshFromOctree(c->root.get(), c->vertexBuffer, c->indexBuffer);

    std::cout << "Mesh generated for chunk " << c->coord.x << "," << c->coord.y << "," << c->coord.z << "\n";
    c->state = ChunkState::Ready;
    */

    /*
    bridge->ECSBridge->readyChunks.push(c); // envia para ECS  

    ChunkKey key{ c->coord, c->lod };

    std::lock_guard<std::mutex> lock(bridge->mtx); {
        bridge->pending.erase(key);
    }
    */
}  

// BUILD FUNCTION      
void BuildChunk(std::shared_ptr<Chunk> c, DensityCache& densityCache, Bridge* bridge) {         // WorldChunkStorage& world
    std::cout << "makeChunk start for chunk " << c->coord.x << "," << c->coord.y << "," << c->coord.z << "\n";
    try {
        c->state  = ChunkState::Building; 
        //c->memory = std::make_unique<ChunkMemory>();

        int chunkWorldSize = DataWorld::ChunkWorldSize(c->lod);
        ivec3 worldMin = c->coord * chunkWorldSize;              
         
        BuildCxt* ctx = new BuildCxt(worldMin, c->lod, &densityCache);
         
        //ASYNC  
        MultiBuilder::BuildOctreeParallel(
            worldMin, chunkWorldSize, ctx,
            // ~Lambda
            [c, bridge](DCNode* node)
            {
                submitJobChunk(c, node, bridge);
            }
        );

    }

    // CATCHS
    catch (const std::exception& e) {
        std::cerr << "Exception in makeChunk(): " << e.what() << "\n";
        c->state = ChunkState::Empty;
        //c->root.reset();
    }
    catch (...) {
        std::cerr << "Unknown crash in makeChunk() for chunk "
            << c->coord.x << "," << c->coord.y << "," << c->coord.z << "\n";
        c->state = ChunkState::Empty;
        //c->root.reset();
    }

}

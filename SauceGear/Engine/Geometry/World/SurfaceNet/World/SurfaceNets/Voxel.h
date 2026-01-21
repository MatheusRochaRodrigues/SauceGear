#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../../../../../Graphics/ComputeShader.h" 
#include "SurfaceNetsGPU.h"   
#include "MapGenerator.h"    
#include "GSurfPool.h"    
#include "../../WorldOctree/SurfaceNets/WorldSys.h"    

#include"../../../../../ECS/Systems/DebugRenderer.h"

class voxelSystem {
public: 
    voxelSystem() { 
        generator =     new GPUMapGenerator ("Map/VoxelMap.comp");  
        computeShader = new ComputeShader   ("SurfaceNet/SurfaceNetsGPU.comp");   
    }

    /*
    std::vector<Chunk*> gnrtChunk() {
        glm::vec3 numChunks = sysv.numChunksPerAxis; 

        std::vector<Chunk*> worldChunks;
        worldChunks.resize(int(numChunks.x * numChunks.y * numChunks.z)); // pré-aloca espaço suficien
        
        //SurfaceNetsGPU::SurfaceNetsGPUBuffer cBuff;

        int index = 0;
        for (int cz = 0; cz < numChunks.z; cz++) for (int cy = 0; cy < numChunks.y; cy++) for (int cx = 0; cx < numChunks.x; cx++)
        { 
            //1 - deslocamento (em coordenadas de mundo)
            //glm::vec3 offset = glm::vec3(cx, cy, cz) * sysv.get_chunkSize(); 
            //glm::vec3 offset = glm::vec3(cx, cy, cz) * sysv.get_chunkSize() - glm::vec3(sysv.get_chunkSize() * 0.5f, 0, sysv.get_chunkSize() * 0.5f);
            //glm::vec3 offset = glm::vec3(cx, cy, cz) * (sysv.get_chunkSize() - sysv.get_voxelSize());

            glm::vec3 offset = glm::vec3(cx, cy, cz) * (float)(sysv.get_cellGrid() * sysv.get_voxelSize());

            //2 - respeitar diretamente o tamanho do voxel e da grid
                //glm::vec3 offset = glm::vec3(cx, cy, cz) * (float)(sysv.get_voxelGrid() - 1) * sysv.get_voxelSize(); // Use (voxelGrid - 1) porque o número de células é 1 a menos que o número de pontos (grid = numCells + 1).
             
            auto* voxel = new Chunk();
            voxel->coord = offset; 

            auto& vBuff = *voxel->buff.get();

            std::cout << "Chunk offset: " << offset.x << "," << offset.y << "," << offset.z << "\n";

            // gera SDF no GPU          //CUIDADO BUSQUE ATUALIZAR TODAS SUAS ESTRUTURAS POR SI SO SEMPRE QUE A DIMENSAO FOR ALTERADA
            generator->Generate(offset, vBuff); // dim=64, voxelSize=1.0

            size_t dim = sysv.get_voxelGrid();
            size_t expected = dim * dim * dim;
            std::cout << "dim = " << dim << " expected = " << expected
                << " buff.density.size() = " << vBuff.densityMap.size() << std::endl;


            //vBuff.densityMap = GeneratorMap::GenerateSphereSDF(sysv.get_cellGrid(), 10);

            std::cout << "p 3" << std::endl;
            
            voxel->mesh = SurfaceNetsGPU::Generate(vBuff, offset, computeShader->ID, cBuff, 0);
            worldChunks[index++] = voxel;  
        }
        std::cout << "p 5" << std::endl;

        return worldChunks; // retorna o vetor já totalmente preenchido
    }
    */
    std::vector<std::pair<Chunk*, OctreeNode*>> gnrtChunk() {
        auto* world = new WorldSys(generator, computeShader);
        
        DebugRenderer::Point(world->octree->root->getBounds().min, glm::vec3(0,0,1.0f), 15.0f, DebugPointType::Circle, true);
        DebugRenderer::Point(world->octree->root->getBounds().max, glm::vec3(0,0,1.0f), 15.0f, DebugPointType::Square, true); 

        DebugRenderer::Cube(
            world->octree->root->getBounds().min,
            world->octree->root->getBounds().max,
            glm::vec3(0, 1, 0),
            true   // Unity-style: redesenha todo frame
        );
         
        world->Update(glm::vec3(0, 0, 0));
        auto allChunks = world->CollectChunks();
        std::cout << " fd " << allChunks.size() << std::endl;

        return allChunks;

        /*
        glm::vec3 numChunks = sysv.numChunksPerAxis;

        std::vector<Chunk*> worldChunks;
        //worldChunks.resize(int(numChunks.x * numChunks.y * numChunks.z)); // pré-aloca espaço suficien

        //SurfaceNetsGPU::SurfaceNetsGPUBuffer cBuff;

        int index = 0;
        for (int cz = 0; cz < numChunks.z; cz++) for (int cy = 0; cy < numChunks.y; cy++) for (int cx = 0; cx < numChunks.x; cx++)
        {
            std::cout << std::endl;
            //1 - deslocamento (em coordenadas de mundo)
            //glm::vec3 offset = glm::vec3(cx, cy, cz) * sysv.get_chunkSize(); 
            //glm::vec3 offset = glm::vec3(cx, cy, cz) * sysv.get_chunkSize() - glm::vec3(sysv.get_chunkSize() * 0.5f, 0, sysv.get_chunkSize() * 0.5f);
            //glm::vec3 offset = glm::vec3(cx, cy, cz) * (sysv.get_chunkSize() - sysv.get_voxelSize());

            //glm::vec3 offset = glm::vec3(cx, cy, cz) * (float)(sysv.get_cellGrid() - 1) * sysv.get_voxelSize(); 
            

            //glm::vec3 offset = glm::vec3(cx, cy, cz) * ((float)sysv.get_cellGrid() * sysv.get_voxelSize());
            //glm::vec3 offset = glm::vec3(cx, cy, cz) * (((float)sysv.get_cellGrid() - 1) * sysv.get_voxelSize());
            glm::vec3 offset = glm::vec3(cx, cy, cz) * ( ((float)sysv.get_cellGrid() * sysv.get_voxelSize()) - sysv.get_voxelSize());



            //2 - respeitar diretamente o tamanho do voxel e da grid
                //glm::vec3 offset = glm::vec3(cx, cy, cz) * (float)(sysv.get_voxelGrid() - 1) * sysv.get_voxelSize(); // Use (voxelGrid - 1) porque o número de células é 1 a menos que o número de pontos (grid = numCells + 1).

            auto* voxel = new Chunk();
            voxel->coord = offset;

            auto& vBuff = *voxel->buff.get();

            std::cout << "Chunk offset: " << offset.x << "," << offset.y << "," << offset.z << "\n";
              
            // Acquire a GPU buffer from global pool
            SurfaceNetsGPUBuffer* gpuBuf = GlobalSurfaceNetsPool::Get().Acquire();
            // Make sure capacity is available (must run on GL context thread)
            gpuBuf->ensureCapacity();

            std::cout << " 1p " << gpuBuf->ssboSDF << std::endl;

            // 1) generate SDF on GPU and copy into gpuBuf.ssboSDF (leaveSdfOnGpu=true)
            generator->Generate(offset, vBuff, *gpuBuf, sysv.get_voxelGrid(), sysv.get_voxelSize());

            //vBuff.densityMap = GeneratorMap::GenerateSphereSDF(sysv.get_cellGrid(), 10);  
            voxel->mesh = SurfaceNetsGPU::Generate(vBuff, offset, computeShader->ID, *gpuBuf, true);

            GlobalSurfaceNetsPool::Get().Release(gpuBuf);

            if (voxel->mesh == nullptr) continue; 

            worldChunks.push_back(voxel);
            //worldChunks[index++] = voxel;
            std::cout << "Chunk " << cx << "," << cy << "," << cz << " minSDF=" << *std::min_element(vBuff.densityMap.begin(), vBuff.densityMap.end()) << std::endl;
            std::cout << "Generating " << int(numChunks.x * numChunks.y * numChunks.z) << " chunks\n";
            std::cout << "Chunk done: " << index << " vtx=" << voxel->mesh->vertices.size() << " idx=" << voxel->mesh->indices.size() << std::endl;
              
            //glMemoryBarrier(GL_ALL_BARRIER_BITS);  //glFinish(); // temporário — força GPU a terminar antes de continuar 
        }
        std::cout << "p 5" << std::endl;

        return worldChunks; // retorna o vetor já totalmente preenchido
        */
    }


    /*
    std::vector<Chunk*> gnrtChunk() {
        glm::ivec3 numChunks = glm::ivec3(sysv.numChunksPerAxis);
        std::vector<Chunk*> worldChunks;
        worldChunks.reserve(numChunks.x * numChunks.y * numChunks.z);
         
        for (int cz = 0; cz < numChunks.z; ++cz) for (int cy = 0; cy < numChunks.y; ++cy) for (int cx = 0; cx < numChunks.x; ++cx) {
            glm::vec3 offset = glm::vec3(cx, cy, cz) * float(sysv.get_cellGrid() * sysv.get_voxelSize());

            auto* voxel = new Chunk();
            voxel->coord = offset;
            auto& vBuff = *voxel->buff.get();

            // Acquire a GPU buffer from global pool
            SurfaceNetsGPUBuffer* gpuBuf = GlobalSurfaceNetsPool::Get().Acquire();
            // Make sure capacity is available (must run on GL context thread)
            gpuBuf->ensureCapacity();

            std::cout << " 1p " << gpuBuf->ssboSDF << std::endl;

            // 1) generate SDF on GPU and copy into gpuBuf.ssboSDF (leaveSdfOnGpu=true)
            generator->Generate(offset, vBuff, *gpuBuf);

            // 2) schedule the SurfaceNets GPU work asynchronously using ComputeSyncComponent::Request
            GLuint computeProg = computeShader->ID;
            // capture pointers by value
            ComputeSyncComponent::Request([=]() {
                // This lambda runs immediately on caller thread to enqueue GPU work, then sets a fence.
                // We assume this is the GL context thread. If not, you must ensure GL context.
                // Run SurfaceNetsGPU::Generate but WITHOUT synchronous CPU readback.
                // To make it fully async we can:
                //  - run compute that writes positions/normals/indices into gpuBuf
                //  - place a fence
                // For simplicity, we'll call Generate but it will do readback; instead, we can separate
                // compute-dispatch and readback. Here we dispatch compute and leave readback to callback.

                // bind program and uniforms (done inside Generate)
                glUseProgram(computeProg);
                // set uniforms handled inside Generate call below
                // We'll call a variant that only dispatches compute and leaves GPU buffers filled:
                // (I'll implement DispatchOnly below as static helper)
                SurfaceNetsGPU::DispatchOnly(vBuff, offset, computeProg, *gpuBuf);
                // After DispatchOnly executes, ComputeSyncComponent will create a fence and enqueue callback.
                }, [=]() {
                    // onComplete callback -> called when GPU is done (on main GL thread)
                    // Readback mesh from gpuBuf and assign to voxel->mesh
                    voxel->mesh = SurfaceNetsGPU::ReadbackMeshFromBuffer(*gpuBuf, computeProg);
                    // mark and release pool buffer
                    GlobalSurfaceNetsPool::Get().Release(gpuBuf);
                });

                worldChunks.push_back(voxel);
        }

        return worldChunks;
    }
    */




private:
    ComputeShader* computeShader;
    GPUMapGenerator* generator;
};

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../Graphics/ComputeShader.h" 
#include "../Geometry/World/SurfaceNets/SurfaceNetsGPU.h"   
#include "../Geometry/World/SurfaceNets/MapGenerator.h"    
 
class voxelSystem {
public:
    GPUMapGenerator* generator;  

    voxelSystem() {
        std::cout << "o - 1" << std::endl;
        generator = new GPUMapGenerator("Map/VoxelMap.comp"); 
        std::cout << "o - 4" << std::endl;
        computeShader = new ComputeShader("SurfaceNet/SurfaceNetsGPU.comp");  

        std::cout << "o - 6" << std::endl;
    }

    std::vector<Chunk*> gnrtChunk() {
        glm::vec3 numChunks = sysv.numChunksPerAxis; 

        std::vector<Chunk*> worldChunks;
        worldChunks.resize(int(numChunks.x * numChunks.y * numChunks.z)); // pré-aloca espaço suficien
         
        int index = 0;
        for (int cz = 0; cz < numChunks.z; ++cz) for (int cy = 0; cy < numChunks.y; ++cy) for (int cx = 0; cx < numChunks.x; ++cx)
        {
            std::cout << "p 1" << std::endl;
            glm::vec3 offset = glm::vec3(cx, cy, cz) * (sysv.get_voxelGrid() * sysv.get_voxelSize() * sysv.get_chunkSize());

            Chunk* voxel = new Chunk();
            auto& vBuff = *voxel->buff.get();
            voxel->coord = offset; 

            std::cout << "p 2" << std::endl;
             //CUIDADO BUSQUE ATUALIZAR TODAS SUAS ESTRUTURAS POR SI SO SEMPRE QUE A DIMENSAO FOR ALTERADA
            //generator->Generate(offset, vBuff); // dim=64, voxelSize=1.0
            
            std::cout << "p 3" << std::endl;
            
            vBuff.density = GeneratorMap::GenerateSphereSDF(sysv.get_voxelGrid(), 10);
            voxel->mesh = SurfaceNetsGPU::Generate(vBuff, offset, computeShader->ID, 0);
            worldChunks[index++] = voxel; 

            std::cout << "p 4" << std::endl;
        }
        std::cout << "p 5" << std::endl;

        return worldChunks; // retorna o vetor já totalmente preenchido
    }

private:
    ComputeShader* computeShader;
};

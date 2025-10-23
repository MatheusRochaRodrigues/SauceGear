#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../Graphics/ComputeShader.h" 
#include "../Geometry/World/SurfaceNets/SurfaceNetsGPU.h"   
#include "../Geometry/World/SurfaceNets/MapGenerator.h"    
 
class voxelSystem {
public: 
    voxelSystem() { 
        generator =     new GPUMapGenerator ("Map/VoxelMap.comp");  
        computeShader = new ComputeShader   ("SurfaceNet/SurfaceNetsGPU.comp");   
    }

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

private:
    ComputeShader* computeShader;
    GPUMapGenerator* generator;
};

#pragma once
#include <iostream> 
#include <unordered_map> 
#include <shared_mutex> 
#include <glm/glm.hpp> 
#include "SDFBrick.h"
#include "density.h"
#include "../../../Utils/Math/Morton3D.h"

using namespace glm;

class DensityCache
{
public:

    std::unordered_map<BrickKey, std::shared_ptr<SDFBrick>, BrickHasher> bricks;
    std::shared_mutex mutex;

    float Sample(const ivec3& worldPos, int lod = 0)
    {
        // int stride = 1 << lod;

        //ivec3 brickCoord = worldPos / BRICK_SIZE;
        ivec3 brickCoord = ivec3(floor(vec3(worldPos) / float(BRICK_SIZE)));

        std::shared_ptr<SDFBrick> brick;

        {
            std::shared_lock lock(mutex);   // only read

            auto it = bricks.find({ brickCoord });

            if (it != bricks.end())
                brick = it->second;
        }

        if (!brick)
            if (lod > 1)
                return Density_Func(worldPos);
            else
                brick = BuildBrick(brickCoord);
         
         
        ivec3 local = worldPos - (brickCoord * BRICK_SIZE);     //ivec3 local = worldPos % BRICK_SIZE; // -> % com negativo quebra

        if (local.x < 0 || local.y < 0 || local.z < 0 ||
            local.x >= BRICK_SIZE || local.y >= BRICK_SIZE || local.z >= BRICK_SIZE)
        {
            std::cerr << "Local coord out of bounds: " << local.x << ", " << local.y << ", " << local.z << std::endl;
            assert(false && "Local coord out of bounds");
        }
         
        return brick->density[index3D(local.x, local.y, local.z)];

        /*return brick->density
            [local.x]
            [local.y]
            [local.z];*/


        /*
        if (!brick->valid[x][y][z]) {
            brick->density[x][y][z] = Density_Func(p);
            brick->valid[x][y][z] = true;
        }
        */

    }

    inline uint32_t index3D(int x, int y, int z) {
        return Morton3D(x, y, z);
    }

    /*static DensityCache& Get()
    {
        static DensityCache instance;
        return instance;
    }*/

private:

    std::shared_ptr<SDFBrick> BuildBrick(const ivec3& coord)
    {
        auto brick = std::make_shared<SDFBrick>();

        ivec3 worldBase = coord * BRICK_SIZE;

        for (int z = 0; z <= BRICK_SIZE; z++)
            for (int y = 0; y <= BRICK_SIZE; y++)
                for (int x = 0; x <= BRICK_SIZE; x++)
                {
                    ivec3 p = worldBase + ivec3(x, y, z);
                     
                    uint32_t idx = index3D(x, y, z);
                    brick->density[idx] = Density_Func(p);       //brick->density[x][y][z] = Density_Func(p);
                }

        {
            std::unique_lock lock(mutex);   // only writhe

            // this brick has already been created by another thread
            auto it = bricks.find({ coord });
            if (it != bricks.end()) return it->second; 

            bricks[{coord}] = brick;
        }

        return brick;
    }

}; 




//1 option
/*
struct BuildContext
{
    DensityCache* density;
    int chunkLOD;
};
*/



/*
void BuildSDFChunk(SDFGrid& grid, ivec3 origin, int lod)
{
    int voxelSize = 1 << lod;
    int res = (CHUNK_SIZE >> lod) + 1;

    for(int z=0; z<res; z++)
    for(int y=0; y<res; y++)
    for(int x=0; x<res; x++)
    {
        vec3 worldPos =
            origin +
            vec3(x,y,z) * voxelSize;

        grid[x + y*res + z*res*res] =
            Density(worldPos);
    }
}
*/













/*
void BuildSDF()
{
    int stride = 1 << lod;

    int resolution = (CHUNK_SIZE / stride) + 1;

    sdf = std::make_unique<SDFGrid>();
    sdf->resolution = resolution;
    sdf->cellSize = BASE_CELL_SIZE * stride;
    sdf->values.resize(resolution * resolution * resolution);

    ivec3 worldMin = coord * ChunkWorldSize();

    for (int z = 0; z < resolution; z++)
        for (int y = 0; y < resolution; y++)
            for (int x = 0; x < resolution; x++)
            {
                vec3 pos =
                    vec3(worldMin) +
                    vec3(x, y, z) * sdf->cellSize;

                sdf->values[sdf->Index(x, y, z)] = Density(pos);
            }
}


*/
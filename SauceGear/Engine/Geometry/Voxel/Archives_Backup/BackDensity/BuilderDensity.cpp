//#pragma once
//#include <iostream>
//#include "../../World/managerWorld.h"
//#include "density.h"
//#include "SDFGrid.h"

/*
std::unique_ptr<SDFGrid> BuildSDF(ivec3 coord, int lod)
{
    int stride = 1 << lod;

    int resolution = (CHUNK_SIZE / stride) + 1;

    std::unique_ptr<SDFGrid> sdf = std::make_unique<SDFGrid>();
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

    return sdf;
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
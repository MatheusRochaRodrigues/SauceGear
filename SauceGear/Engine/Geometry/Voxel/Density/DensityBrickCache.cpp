#include "DensityBrickCache.h"



/*
blockCoord = floor(worldPos / blockSize)
block = cache.get(blockCoord)

local = worldPos % blockSize

return block->sdf[local]




SampleSDF(worldPos)
*/
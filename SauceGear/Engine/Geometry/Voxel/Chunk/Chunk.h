#pragma once 
#include "../Octree/DCNode.h"

class Chunk
{
public:
	glm::ivec3 origin;
	int size;
	int lod;

	DCNode* octree;

	void Build();
	void Destroy();
};

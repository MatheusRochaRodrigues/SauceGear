#pragma once
#include "DCNode.h"

DCNode* BuildOctree(const glm::ivec3& min, int size);
DCNode* ConstructOctreeNodes(DCNode* node);
void DestroyOctree(DCNode* node);

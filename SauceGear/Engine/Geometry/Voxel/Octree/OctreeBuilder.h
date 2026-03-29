#pragma once
#include "DCNode.h" 

DCNode* BuildOctree(const glm::ivec3 min, int size, BuildContext_CK& ctx);
bool NodeHasSurface(glm::ivec3 min, int size, BuildContext_CK& ctx);
DCNode* ConstructOctreeNodes(DCNode* node, BuildContext_CK& ctx);
void DestroyOctree(DCNode* node);

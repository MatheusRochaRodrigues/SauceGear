#pragma once
#include "DCNode.h" 

DCNode* BuildOctree(const glm::ivec3 min, int size, BuildContext_CK& ctx);
bool NodeHasSurface(glm::vec3 min, int size, BuildContext_CK& ctx);		//glm::ivec3 min
DCNode* ConstructOctreeNodes(DCNode* node, BuildContext_CK& ctx);
void DestroyOctree(DCNode* node);

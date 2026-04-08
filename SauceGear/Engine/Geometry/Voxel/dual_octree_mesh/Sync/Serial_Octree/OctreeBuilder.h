#pragma once
#include <glm/glm.hpp>

class DCNode;
struct BuildCxt;

bool NodeHasSurface(glm::vec3 min, float size, BuildCxt& ctx);
DCNode* BuildOctree(const glm::ivec3 min, int size, BuildCxt& ctx);
DCNode* ConstructOctreeNodes( DCNode* node, BuildCxt& ctx );

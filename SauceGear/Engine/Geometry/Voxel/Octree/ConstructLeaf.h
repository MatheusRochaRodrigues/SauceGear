#pragma once 
#include <glm/glm.hpp>

class DCNode;

bool NodeHasSurface(glm::ivec3 min, int size);
DCNode* ConstructLeaf(DCNode* leaf);
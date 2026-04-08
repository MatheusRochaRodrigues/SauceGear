#pragma once 
#include <glm/glm.hpp>

class DCNode;
class BuildCxt;

namespace MultiBuilder {
	DCNode* ConstructLeaf(DCNode* leaf, BuildCxt&);
}
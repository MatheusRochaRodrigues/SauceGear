#pragma once
#include <functional>
#include <glm/glm.hpp>  
//Threads
#include "../../Data/DCNode.h" 
#include "../../../../../Utils/Threads/ThreadArena.h"  
#include "../../../../../Utils/Threads/JobSystem.h"

namespace MultiBuilder {
	void BuildOctreeParallel(const glm::ivec3 min, int size, BuildCxt* ctx, std::function<void(DCNode*)>);
}
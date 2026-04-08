#pragma once 
#include "../../Data/DCNode.h"
#include "../../../../../Graphics/Vertex.h"

namespace SerialBuilder { 
	void GenerateMeshFromOctree(
		DCNode* root,
		VertexBuffer& vertices,
		IndexBuffer& indices
	);

	// without any logic for creation octree  
}
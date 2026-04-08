#pragma once 
#include "../../Data/DCNode.h"
#include "../../../../../Graphics/Vertex.h"

namespace MultiBuilder {

	void GenerateMeshFromOctree_MultiThread(
		DCNode* root,
		VertexBuffer& vertices,
		IndexBuffer& indices
	);

	// without any logic for creating octree.
}
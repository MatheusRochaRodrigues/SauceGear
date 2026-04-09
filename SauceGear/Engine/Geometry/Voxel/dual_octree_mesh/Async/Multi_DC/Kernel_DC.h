#pragma once  
#include "../../../../../Graphics/Vertex.h"

class DCNode; 

namespace MultiBuilder {

	void GenerateMeshFromOctree_MultiThread(
		DCNode* root,
		VertexBuffer& vertices,
		IndexBuffer& indices
	);

	// without any logic for creating octree.
}
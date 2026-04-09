#pragma once  
#include "../../../../../Graphics/Vertex.h"

class DCNode;

namespace MultiBuilder {

	constexpr int MESH_TASK_SIZE_THRESHOLD = 16;

	void GenerateVertexIndices(DCNode* node, VertexBuffer& localVertexBuffer);
	void ContourCellProc(DCNode* node, IndexBuffer& indexBuffer);
	 
}
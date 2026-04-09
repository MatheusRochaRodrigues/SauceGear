#pragma once
#include "../../../../../Graphics/Vertex.h"

class DCNode;

struct MeshChunk
{
	VertexBuffer vb;
	IndexBuffer ib;

	void Reserve()
	{
		vb.reserve(1024);
		ib.reserve(2048);
	}
};

struct MeshTaskData
{
	DCNode* node;
	MeshChunk* chunk;
};



 
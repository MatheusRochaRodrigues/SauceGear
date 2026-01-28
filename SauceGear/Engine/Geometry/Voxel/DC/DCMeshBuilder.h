#pragma once 
#include "../Octree/DCNode.h"
#include "../../../Graphics/Vertex.h"

void GenerateMeshFromOctree(
	DCNode* root,
	VertexBuffer& vertices,
	IndexBuffer& indices
);

//Sem nenhuma lógica de octree creation.
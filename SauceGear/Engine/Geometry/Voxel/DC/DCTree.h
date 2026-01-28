#pragma once 
#include "QEF/qef.h"
#include "../../../Assets/MeshAsset.h" 
#include <glm/glm.hpp> 
using namespace glm;

// -------------------------------Structs--------------------------------------
enum OctreeNodeType
{
	Node_None,
	Node_Internal,
	Node_Psuedo,
	Node_Leaf,
};

struct OctreeDrawInfo
{
	OctreeDrawInfo() : index(-1), corners(0) {}

	int				index;
	int				corners;
	vec3			position;
	vec3			averageNormal;
	svd::QefData	qef;
};
// ----------------------------------------------------------------------------

class DCNode
{
public:

	DCNode()
		: type(Node_None)
		, min(0, 0, 0)
		, size(0)
		, drawInfo(nullptr)
	{
		for (int i = 0; i < 8; i++) children[i] = nullptr;
	}

	DCNode(const OctreeNodeType _type)
		: type(_type)
		, min(0, 0, 0)
		, size(0)
		, drawInfo(nullptr)
	{
		for (int i = 0; i < 8; i++)
		{
			children[i] = nullptr;
		}
	}

	OctreeNodeType	type;
	ivec3			min;
	int				size;
	DCNode* children[8];
	OctreeDrawInfo* drawInfo;
};

DCNode* BuildOctree(const ivec3& min, const int size, const float threshold);
void DestroyOctree(DCNode* node);
void GenerateMeshFromOctree(DCNode* node, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);



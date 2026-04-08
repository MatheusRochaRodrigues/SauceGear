#include "OctreeBuilder.h"
#include "../../Data/DCNode.h" 
#include "ConstructLeaf.h"
#include "../../../Density/Density.h"
#include "../../../Density/DensityBrickCache.h"
#include "../../../../../World/WorldController.h"
 
bool NodeHasSurface(vec3 min, float size, BuildCxt& ctx)
{
	const float d = Density_Func(min + (size / 2.0f));
	const float surfaceNetThreshold = size * 2 * 2.25f;
	return std::abs(d) < surfaceNetThreshold;
}

DCNode* BuildOctree (
	/*Parameters World*/
	const ivec3 min, const int size, 
	/*Data Chunk*/
	BuildCxt& ctx
) {
	DCNode* root = new DCNode;
	root->min = min;
	root->size = size;
	root->type = Node_Internal; 

	if (ConstructOctreeNodes(root, ctx) == nullptr)
		return nullptr; 

	return root;
}

DCNode* ConstructOctreeNodes(
	DCNode* node,	//unique Data that is truly recursive
	BuildCxt& ctx
) { 
	if (!node)
	{
		return nullptr;
	}

	float minimunNode = BASE_CELL_SIZE * (1 << ctx.chunkLOD);
	if (node->size == minimunNode)		// node->size == (1 << ctx.chunkLOD)   	// == 1 min size node accept		//	==
	{ 
		return ConstructLeaf(node, ctx);
	}

	if (node->size < minimunNode) assert("NodeSize is smaller than 1");		//node->size < 1

	const float childSize = node->size / 2;
	bool hasChildren = false;

	for (int i = 0; i < 8; i++)
	{ 
		vec3 min = node->min + (CHILD_MIN_OFFSETS_FLOAT[i] * childSize);
		if (!NodeHasSurface(min, childSize, ctx)) continue;

		DCNode* child = new DCNode;
		child->size = childSize;
		child->min = min;
		child->type = Node_Internal; 
		 
		node->children[i] = ConstructOctreeNodes(child, ctx);
		hasChildren |= (node->children[i] != nullptr); 
	}
	 
	if (!hasChildren)
	{
		delete node;
		return nullptr;
	} 

	return node;
}


void DestroyOctree(DCNode* node)
{
	if (!node) return;
	for (int i = 0; i < 8; i++)
		DestroyOctree(node->children[i]);
	delete node;
}




















/*


DCNode* BuildOctree2(const glm::ivec3& min, int size)
{
	DCNode* node = new DCNode;
	node->min = min;
	node->size = size;
	node->type = DCNodeType::Node_Internal;

	if (!NodeHasSurface(min, size))
	{
		delete node;
		return nullptr;
	}

	if (size == 1)
	{
		// Leaf será construída pelo DC
		//return node;

		return ConstructLeaf(node);
	}

	const int childSize = size >> 1;
	for (int i = 0; i < 8; i++)
	{
		glm::ivec3 offset(
			(i & 1) ? childSize : 0,
			(i & 2) ? childSize : 0,
			(i & 4) ? childSize : 0
		);

		node->children[i] = BuildOctree(min + offset, childSize);
	}

	return node;
}

*/
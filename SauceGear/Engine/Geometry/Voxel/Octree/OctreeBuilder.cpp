#include "OctreeBuilder.h"
#include "../Density/Density.h"
#include "ConstructLeaf.h"
#include "../../../World/WorldController.h"
#include "../../Voxel/Density/DensityBrickCache.h"

DCNode* BuildOctree (
	/*Parameters World*/
	const ivec3 min, const int size, 
	/*Data Chunk*/
	BuildContext_CK& ctx
) {
	DCNode* root = new DCNode;
	root->min = min;
	root->size = size;
	root->type = Node_Internal; 

	if (ConstructOctreeNodes(root, ctx) == nullptr)
		return nullptr; 

	return root;
}
 
bool NodeHasSurface(vec3 min, int size, BuildContext_CK& ctx)
{ 
	const float d = Density_Func(min + (size / 2.0f)		*  VOXEL_SCALE); 	/// implement voxelSize
	const float surfaceNetThreshold = size * 2 * 2.25f;
	return std::abs(d) < surfaceNetThreshold;
}

DCNode* ConstructOctreeNodes(
	DCNode* node,	//unique Data that is truly recursive
	BuildContext_CK& ctx
) { 
	if (!node)
	{
		return nullptr;
	}

	if (node->size == (BASE_CELL_SIZE << ctx.chunkLOD))		// node->size == (1 << ctx.chunkLOD)   	// == 1 min size node accept		//	==
	{ 
		return ConstructLeaf(node, ctx);
	}

	if (node->size < 1) assert("NodeSize is smaller than 1");

	const int childSize = node->size / 2;
	bool hasChildren = false;

	for (int i = 0; i < 8; i++)
	{ 
		ivec3 min = node->min + (CHILD_MIN_OFFSETS[i] * childSize);  
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
#include "OctreeBuilder.h"
#include "../Density/Density.h"
#include "ConstructLeaf.h"

DCNode* BuildOctree(const ivec3& min, const int size)
{
	DCNode* root = new DCNode;
	root->min = min;
	root->size = size;
	root->type = Node_Internal;

	ConstructOctreeNodes(root); 

	return root;
}

DCNode* ConstructOctreeNodes(DCNode* node)
{
	if (!node)
	{
		return nullptr;
	}

	if (node->size == 1)
	{
		return ConstructLeaf(node);
	}

	const int childSize = node->size / 2;
	bool hasChildren = false;

	for (int i = 0; i < 8; i++)
	{
		ivec3 min = node->min + (CHILD_MIN_OFFSETS[i] * childSize);  
		if (!NodeHasSurface(min, childSize)) continue;

		DCNode* child = new DCNode;
		child->size = childSize;
		child->min = min;
		child->type = Node_Internal;
		 
		node->children[i] = ConstructOctreeNodes(child);
		hasChildren |= (node->children[i] != nullptr);
	}

	if (!hasChildren)
	{
		delete node;
		return nullptr;
	}

	return node;
}


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

void DestroyOctree(DCNode* node)
{
	if (!node) return;
	for (int i = 0; i < 8; i++)
		DestroyOctree(node->children[i]);
	delete node;
}

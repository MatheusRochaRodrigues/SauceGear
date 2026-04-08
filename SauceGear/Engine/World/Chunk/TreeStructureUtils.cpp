#pragma once
#include "../../Geometry/Voxel/dual_octree_mesh/Data/DCNode.h" 

DCNode* Prune(DCNode* node)
{
    if (!node) return nullptr;

    if (node->type == DCNodeType::Node_Leaf)
        return node;

    bool hasChildren = false;

    for (int i = 0; i < 8; i++)
    {
        node->children[i] = Prune(node->children[i]);
        hasChildren |= (node->children[i] != nullptr);
    }

    if (!hasChildren)
    {
        delete node;
        return nullptr;
    }

    return node;
} 

//DCNode* Prune(DCNode* node)
//{
//    if (!node) return nullptr;
//
//    bool hasValidChild = false;
//
//    for (int i = 0; i < 8; i++)
//    {
//        node->children[i] = Prune(node->children[i]);
//        if (node->children[i]) hasValidChild = true;
//    }
//
//    if (!hasValidChild) // && !node->hasSurface
//    {
//        delete node;
//        return nullptr;
//    }
//
//    return node;
//}

/*
void PruneOctree(DCNode* node)
{
    if (!node) return;

    for (int i = 0; i < 8; i++)
    {
        PruneOctree(node->children[i]);

        if (node->children[i] && node->children[i]->type == DCNodeType::Node_None)
        {
            delete node->children[i];
            node->children[i] = nullptr;
        }
    }
}
*/
#pragma once 
#include <glm/glm.hpp> 
#include "OctreeDrawInfo.h"
#include "EdgeCache.h"

using namespace glm;

const int MATERIAL_AIR = 0;
const int MATERIAL_SOLID = 1;

const float QEF_ERROR = 1e-6f;
const int QEF_SWEEPS = 4;


// ----------------------------------------------------------------------------
// data from the original DC impl, drives the contouring process

const int edgevmap[12][2] =
{
	{0,4},{1,5},{2,6},{3,7},	// x-axis 
	{0,2},{1,3},{4,6},{5,7},	// y-axis
	{0,1},{2,3},{4,5},{6,7}		// z-axis
};

const int edgemask[3] = { 5, 3, 6 };

const int cellProcEdgeMask[6][5] = { {0,1,2,3,0},{4,5,6,7,0},{0,4,1,5,1},{2,6,3,7,1},{0,2,4,6,2},{1,3,5,7,2} };

const int edgeProcEdgeMask[3][2][5] = {
	{{3,2,1,0,0},{7,6,5,4,0}},
	{{5,1,4,0,1},{7,3,6,2,1}},
	{{6,4,2,0,2},{7,5,3,1,2}},
};

const int processEdgeMask[3][4] = { {3,2,1,0},{7,5,6,4},{11,10,9,8} };
 

// ---------------------------------------------------------------------------- ------------------------
// THIS VECTOR tell diferrent combinations inter nodes of her father
const int cellProcFaceMask[12][3] = { {0,4,0},{1,5,0},{2,6,0},{3,7,0},{0,2,1},{4,6,1},{1,3,1},{5,7,1},{0,1,2},{2,3,2},{4,5,2},{6,7,2} };
//0 = X, 1 = Y, 2 = Z	-> direction



// ---------------------------------------------------------------------------- 
const int faceProcFaceMask[3][4][3] = {
	{{4,0,0},{5,1,0},{6,2,0},{7,3,0}},		// X		
	{{2,0,1},{6,4,1},{3,1,1},{7,5,1}},		// Y
	{{1,0,2},{3,2,2},{5,4,2},{7,6,2}}		// Z
};

const int faceProcEdgeMask[3][4][6] = {
	{{1,4,0,5,1,1},{1,6,2,7,3,1},{0,4,6,0,2,2},{0,5,7,1,3,2}},
	{{0,2,3,0,1,0},{0,6,7,4,5,0},{1,2,0,6,4,2},{1,3,1,7,5,2}},
	{{1,1,0,3,2,0},{1,5,4,7,6,0},{0,1,5,0,4,1},{0,3,7,2,6,1}}
};


const ivec3 CHILD_MIN_OFFSETS[] =
{
	// needs to match the vertMap from Dual Contouring impl
	ivec3(0, 0, 0),		//0
	ivec3(0, 0, 1),		//1
	ivec3(0, 1, 0),		//2
	ivec3(0, 1, 1),		//3
	ivec3(1, 0, 0),		//4
	ivec3(1, 0, 1),		//5
	ivec3(1, 1, 0),		//6
	ivec3(1, 1, 1),		//7
};
 
const vec3 CHILD_MIN_OFFSETS_FLOAT[] =
{
	// needs to match the vertMap from Dual Contouring impl
	vec3(0, 0, 0),		//0
	vec3(0, 0, 1),		//1
	vec3(0, 1, 0),		//2
	vec3(0, 1, 1),		//3
	vec3(1, 0, 0),		//4
	vec3(1, 0, 1),		//5
	vec3(1, 1, 0),		//6
	vec3(1, 1, 1),		//7
};

// -------------------------------Structs--------------------------------------
enum DCNodeType
{
	Node_None,
	Node_Internal,
	Node_Psuedo,
	Node_Leaf,
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
		for (int i = 0; i < 8; i++) 
			children[i] = nullptr;

		childMask = 0;
	}

	DCNode(const DCNodeType _type)
		: type(_type)
		, min(0, 0, 0)
		, size(0)
		, drawInfo(nullptr)
	{
		for (int i = 0; i < 8; i++) 
			children[i] = nullptr; 

		childMask = 0;
	}

	DCNodeType				type;
	vec3					min;
	float					size;
	DCNode*					children[8];
	OctreeDrawInfo*			drawInfo;

	std::atomic<uint8_t>	childMask{ 0 };		// 8 bits -> 1 bit for each child 
};

inline bool IsValidNode(DCNode* n) {
	return 
		n && 
		!(n->childMask == 0 && n->type != DCNodeType::Node_Leaf) && 
		n->type != DCNodeType::Node_None;										 
}   

inline bool hasChildNodes(DCNode* n) {
	return n->childMask != 0;
}

inline void MarkChildLife(DCNode* node, int i) {
	node->childMask.fetch_or(1 << i, std::memory_order_relaxed);
}


/*
inline bool IsValidNode(DCNode* n) {
	return n && n->type != DCNodeType::Node_None;	//Node_Empty
}
*/


// ============================================================================
// CHUNK CONTEXT
// ============================================================================

class DensityCache; 
//struct ChunkMemory;

struct BuildCxt
{
	const ivec3		minOctree;
	const int		chunkLOD;
	DensityCache*	densityCache;
	EdgeCache		edgeCache;
	// Thread
	//ChunkMemory*	memory;  

	//NO THREAD VERSION
	BuildCxt(const ivec3 minOctree, const int chunkLOD, DensityCache* densityCache) :
		minOctree(minOctree), chunkLOD(chunkLOD), densityCache(densityCache) {};
};
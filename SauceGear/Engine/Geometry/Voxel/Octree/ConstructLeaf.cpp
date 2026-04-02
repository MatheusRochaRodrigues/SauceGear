#include "ConstructLeaf.h"
#include "DCNode.h" 
#include "../Density/DensityBrickCache.h"
#include "EdgeCache.h"

using namespace glm;  
 
vec3 CalculateSurfaceNormal(const vec3& p)
{
	const float H = 0.001f;
	const float dx = Density_Func(p + vec3(H, 0.f, 0.f)) - Density_Func(p - vec3(H, 0.f, 0.f));
	const float dy = Density_Func(p + vec3(0.f, H, 0.f)) - Density_Func(p - vec3(0.f, H, 0.f));
	const float dz = Density_Func(p + vec3(0.f, 0.f, H)) - Density_Func(p - vec3(0.f, 0.f, H));

	return glm::normalize(vec3(dx, dy, dz));
}  

// y = y0​ * (1−t) + y1 ​* t =>         Interpolation basic        being T that interpolates where it will be on the edge
static vec3 FindSurfaceEdgeIntersection_ZeroCrossing(uint32_t c1, uint32_t c2, float v1, float v2, const int size) {
	float interp1 = v1 / (v1 - v2);         // t            ==   Acha onde cruza a superfície em 0
	float interp2 = 1.0f - interp1;         // 1- t
	return interp2 * glm::vec3(CHILD_MIN_OFFSETS[c1] * size) + interp1 * glm::vec3(CHILD_MIN_OFFSETS[c2] * size);
} 


EdgeIntersection& SampleEdge(const ivec3& p, int axis, BuildContext_CK& ctx )
{
	//ivec3 localVoxelMin = p;
	/*if (localVoxelMin.x < 0 || localVoxelMin.x >= ctx.edgeCache.resolution ||
		localVoxelMin.y < 0 || localVoxelMin.y >= ctx.edgeCache.resolution ||
		localVoxelMin.z < 0 || localVoxelMin.z >= ctx.edgeCache.resolution)
	{
		std::cout	<< " x " << localVoxelMin.x
					<< " y " << localVoxelMin.y
					<< " z " << localVoxelMin.z
					<< " incorrect resolution " << ctx.edgeCache.resolution << std::endl;

		assert(false && "out of bounds");
	}*/

	uint32_t idx = ctx.edgeCache.Index(p.x, p.y, p.z);		 
	EdgeIntersection* edge;

	if      (axis == 0) edge = &ctx.edgeCache.edgesX[idx];
	else if (axis == 1) edge = &ctx.edgeCache.edgesY[idx];
	else                edge = &ctx.edgeCache.edgesZ[idx];

	//if (edge->valid) return edge;

	// -------- COMPUTE --------
	// Zero Crossing Surface
	/*ivec3 p1 = p;
	ivec3 p2 = p + ivec3(axis == 0, axis == 1, axis == 2);

	float d1 = ctx.densityCache->Sample(p1, 1);
	float d2 = ctx.densityCache->Sample(p2, 1);

	float t = d1 / (d1 - d2);

	vec3 pos = vec3(p1) + t * vec3(p2 - p1);

	// Normal Gradient
	vec3 normal = CalculateSurfaceNormal(pos);

	edge->position = pos;
	edge->normal = normal;
	edge->valid = 1;

	return *edge;*/

	return *edge;
}

int GetEdgeAxis(int c1, int c2)
{
	ivec3 p1 = CHILD_MIN_OFFSETS[c1];
	ivec3 p2 = CHILD_MIN_OFFSETS[c2];

	ivec3 delta = p2 - p1;

	if (delta.x != 0) return 0;	//axis X
	if (delta.y != 0) return 1;	//axis Y
	return 2;				  	//axis Z
}

DCNode* ConstructLeaf(DCNode* leaf, BuildContext_CK& ctx) {
	if (!leaf)  return nullptr;			// || leaf->size != 1

	float	cornerDensity[8];		
	int		corners = 0;				// sinais dos 8 cantos
	int		cornersNegative = 0; 

	float voxelSize = VOXEL_SCALE;	//DataWorld::getVoxelSize(ctx.chunkLOD)

	for (int i = 0; i < 8; i++)
	{
		//const ivec3 cornerPos = leaf->min + CHILD_MIN_OFFSETS[i] * leaf->size;
		//cornerDensity[i] = ctx.densityCache->Sample(cornerPos, leaf->size);			// Density_Func(vec3(cornerPos));

		ivec3 corner2Grid = leaf->min + CHILD_MIN_OFFSETS[i] * leaf->size; // grid 
		vec3 corner2World = vec3(corner2Grid) * voxelSize;			/// implement voxelSize			
		cornerDensity[i] = Density_Func(corner2World);			//cornerDensity[i] = ctx.densityCache->Sample(worldPos);  

		if (cornerDensity[i] < 0.0f) ++cornersNegative; 

		const int material = cornerDensity[i] < 0.f ? MATERIAL_SOLID : MATERIAL_AIR;
		corners |= (material << i);
	}

	// voxel is full inside or outside the volume -> //void block 
	if (cornersNegative == 0 || cornersNegative == 8) {
		delete leaf;
		return nullptr;
	}

	// otherwise the voxel contains the surface, so find the edge intersections
	const int MAX_CROSSINGS = 6;
	int edgeCount = 0;
	vec3 averageNormal(0.0f);
	svd::QefSolver qef; 

	for (auto& edge : edgevmap) {
		if (edgeCount >= MAX_CROSSINGS) break;

		const int c1 = edge[0];	  
		const int c2 = edge[1];
		float d1 = cornerDensity[c1];
		float d2 = cornerDensity[c2];	


		// zero crossing on this edge
		if ((d1 < 0.0f) != (d2 < 0.0f)) {
			vec3 local = FindSurfaceEdgeIntersection_ZeroCrossing(c1, c2, d1, d2, leaf->size);	// local
			vec3 p = (glm::vec3(leaf->min) + local) * voxelSize;				/// implement voxelSize
			const vec3 n = CalculateSurfaceNormal(p); 

			//------------------- 2 option ----------------------------
			/*
			vec3 p, n;  
			ivec3 localVoxelMin = (leaf->min - ctx.minOctree) / leaf->size;	// voxelSize == leaf->size	//divisão por voxelSize normaliza o LOD

			int axis = GetEdgeAxis(c1, c2);
			EdgeIntersection& edgeData = SampleEdge(localVoxelMin + CHILD_MIN_OFFSETS[c1], axis, ctx); 

			if (edgeData.valid) {
				p = edgeData.position;		
				n = edgeData.normal;
			} else {  
				p = FindSurfaceEdgeIntersection_ZeroCrossing(c1, c2, d1, d2, leaf->size);
				p = glm::vec3(leaf->min) + p;
				n = CalculateSurfaceNormal(p);

				edgeData.position = p;
				edgeData.normal   = n;
				edgeData.valid    = 1;
			}
			*/
			//---------------------------------------------------------

			qef.add(p.x, p.y, p.z, n.x, n.y, n.z);
			averageNormal += n;		edgeCount++;
		}
	}

	svd::Vec3 qefPosition;
	qef.solve(qefPosition, QEF_ERROR, QEF_SWEEPS, QEF_ERROR);

	OctreeDrawInfo* drawInfo = new OctreeDrawInfo;
	drawInfo->position = vec3(qefPosition.x, qefPosition.y, qefPosition.z);
	drawInfo->qef = qef.getData();
	drawInfo->corners = corners;

	const vec3 min = vec3(leaf->min) * voxelSize;								/// implement voxelSize
	const vec3 max = vec3(leaf->min + ivec3(leaf->size)) * voxelSize;			/// implement voxelSize
	if (drawInfo->position.x < min.x || drawInfo->position.x > max.x ||
		drawInfo->position.y < min.y || drawInfo->position.y > max.y ||
		drawInfo->position.z < min.z || drawInfo->position.z > max.z)
	{
		const auto& mp = qef.getMassPoint();
		drawInfo->position = vec3(mp.x, mp.y, mp.z);
	}

	drawInfo->averageNormal = glm::normalize(averageNormal / (float)edgeCount);
	for (int i = 0; i < 8; i++) drawInfo->cornersDens[i] = cornerDensity[i];

	leaf->type = Node_Leaf;
	leaf->drawInfo = drawInfo;

	return leaf;
}

















//bool NodeHasSurface(const ivec3& min, int size, SDFGrid* sdf)
//{
//	int stride = size / sdf->cellSize;
//
//	float minV = 1e9;
//	float maxV = -1e9;
//
//	for (int z = 0; z <= 1; z++)
//		for (int y = 0; y <= 1; y++)
//			for (int x = 0; x <= 1; x++)
//			{
//				int gx = (min.x / sdf->cellSize) + x * stride;
//				int gy = (min.y / sdf->cellSize) + y * stride;
//				int gz = (min.z / sdf->cellSize) + z * stride;
//
//				float v = sdf->Sample(gx, gy, gz);
//
//				minV = std::min(minV, v);
//				maxV = std::max(maxV, v);
//			}
//
//	return minV <= 0 && maxV >= 0;
//} 




//to analize surface

/*
bool NodeHasSurface(ivec3 min, int size)
{
	//First Way
	const float d = Density_Func(min + (size / 2));
	const float surfaceNetThreshold = size * 2 * 2.25f;
	return std::abs(d) < surfaceNetThreshold;

	//Second Way
	/*float sdfMin = +FLT_MAX;
	float sdfMax = -FLT_MAX;

	for (int i = 0; i < 8; i++)
	{
		const vec3 p = vec3(min + CHILD_MIN_OFFSETS[i] * size);
		const float d = Density_Func(p);
		sdfMin = glm::min(sdfMin, d);
		sdfMax = glm::max(sdfMax, d);
	}

	vec3 center = vec3(min) + vec3(size) * 0.5f;

	float dcenter = Density_Func(center);
	sdfMin = glm::min(sdfMin, dcenter);
	sdfMax = glm::max(sdfMax, dcenter);

	// Se o zero NÃO está no intervalo, não existe superfície
	return !(sdfMin > 0.f || sdfMax < 0.f); */

	//Tercery Way
	/*float sdfMin = +FLT_MAX;
	float sdfMax = -FLT_MAX;

	static const vec3 faceOffsets[6] = {
		{0.5f, 0.5f, 0.0f},
		{0.5f, 0.5f, 1.0f},
		{0.5f, 0.0f, 0.5f},
		{0.5f, 1.0f, 0.5f},
		{0.0f, 0.5f, 0.5f},
		{1.0f, 0.5f, 0.5f}
	};

	for (int i = 0; i < 6; i++)
	{
		vec3 p = vec3(min) + faceOffsets[i] * float(size);
		float d = Density_Func(p);
		sdfMin = std::min(sdfMin, d);
		sdfMax = std::max(sdfMax, d);
	}

	vec3 center = vec3(min) + vec3(size) * 0.5f;

	float dcenter = Density_Func(center);
	sdfMin = glm::min(sdfMin, dcenter);
	sdfMax = glm::max(sdfMax, dcenter);

	// Se o zero NÃO está no intervalo, não existe superfície
	return !(sdfMin > 0.f || sdfMax < 0.f);  */

	// 4 Way
	/*bool hasEdgeCrossing = false;

	for (int e = 0; e < 12; e++)
	{
		vec3 p0 = edgeStart(e);
		vec3 p1 = edgeEnd(e);

		float d0 = Density_Func(p0);
		float d1 = Density_Func(p1);

		if ((d0 < 0) != (d1 < 0))
		{
			hasEdgeCrossing = true;
			break;
		}
	}  

}
*/


/*
int GetMinVoxelSizeForLOD(const ivec3& min, int size)
{
	// Centro do nó
	const vec3 center = vec3(min) + vec3(size * 0.5f);

	// Distância da câmera (substitua pela sua)
	extern vec3 gCameraPosition;
	const float dist = glm::length(center - gCameraPosition);

	// Exemplo de shells de LOD
	if (dist < 64.0f)   return 1;   // LOD 0
	if (dist < 128.0f)  return 2;   // LOD 1
	if (dist < 256.0f)  return 4;   // LOD 2
	if (dist < 512.0f)  return 8;   // LOD 3

	return 16; // muito longe
}

*/


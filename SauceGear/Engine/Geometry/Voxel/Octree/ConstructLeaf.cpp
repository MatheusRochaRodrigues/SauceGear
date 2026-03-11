#include "ConstructLeaf.h"
#include "DCNode.h"
#include "../Density/Density.h" 
using namespace glm; 

bool NodeHasSurface(ivec3 min, int size)
{ 
	const float d = Density_Func( min + (size/2) );
	const float surfaceNetThreshold = size * 2 * 2.25f;
	return std::abs(d) < surfaceNetThreshold;  
}  
 
vec3 CalculateSurfaceNormal(const vec3& p)
{
	const float H = 0.001f;
	const float dx = Density_Func(p + vec3(H, 0.f, 0.f)) - Density_Func(p - vec3(H, 0.f, 0.f));
	const float dy = Density_Func(p + vec3(0.f, H, 0.f)) - Density_Func(p - vec3(0.f, H, 0.f));
	const float dz = Density_Func(p + vec3(0.f, 0.f, H)) - Density_Func(p - vec3(0.f, 0.f, H));

	return glm::normalize(vec3(dx, dy, dz));
}  

// y = y0​ * (1−t) + y1 ​* t =>         Interpolation basic        being T that interpolates where it will be on the edge
static vec3 FindSurfaceEdgeIntersection_ZeroCrossing(uint32_t c1, uint32_t c2, float v1, float v2) {
	float interp1 = v1 / (v1 - v2);         // t            ==   Acha onde cruza a superfície em 0
	float interp2 = 1.0f - interp1;         // 1- t
	return interp2 * glm::vec3(CHILD_MIN_OFFSETS[c1]) + interp1 * glm::vec3(CHILD_MIN_OFFSETS[c2]);
} 

DCNode* ConstructLeaf(DCNode* leaf) {
	if (!leaf || leaf->size != 1)  return nullptr; 

	float	cornerDensity[8];		
	int		corners = 0;        // sinais dos 8 cantos
	int		cornersNegative = 0;

	for (int i = 0; i < 8; i++)
	{
		const ivec3 cornerPos = leaf->min + CHILD_MIN_OFFSETS[i];
		cornerDensity[i] = Density_Func(vec3(cornerPos));
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

			vec3 p = FindSurfaceEdgeIntersection_ZeroCrossing(c1, c2, d1, d2);
			p = glm::vec3(leaf->min) + p;
			const vec3 n = CalculateSurfaceNormal(p);
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

	const vec3 min = vec3(leaf->min);
	const vec3 max = vec3(leaf->min + ivec3(leaf->size));
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


#pragma once 
#include <vector>
#include <glm/glm.hpp>
#include "../../../../World/WorldController.h"  
#include "../../../../Utils/Math/Morton3D.h"  

using namespace glm;
 
struct EdgeIntersection
{
    vec3 position;
    vec3 normal;
    uint8_t valid = 0;
};

struct EdgeCache
{
    int resolution;

    std::vector<EdgeIntersection> edgesX;
    std::vector<EdgeIntersection> edgesY;
    std::vector<EdgeIntersection> edgesZ;

    EdgeCache() { Init(GRID_RESOLUTION /*CHUNK_SIZE*/); }

    void Init(int res)
    {
        //int size = NextPowerOfTwo(res); int count = size * size * size;
        //if((res & (res - 1)) == 0) assert("morton should be power of two"); // power of two
        
        resolution = res + 1;
        int count = resolution * resolution * resolution;

        edgesX.resize(count);
        edgesY.resize(count);
        edgesZ.resize(count);

        memset(edgesX.data(), 0, count * sizeof(EdgeIntersection));
        memset(edgesY.data(), 0, count * sizeof(EdgeIntersection));
        memset(edgesZ.data(), 0, count * sizeof(EdgeIntersection));
    }

    //inline uint32_t Index(int x, int y, int z) const
    //{
    //    return Morton3D(x, y, z);
    //}

    ////No Morton;
    inline uint32_t Index(int x, int y, int z) const
    {
        int r = resolution /*+1*/;

        if (!(x >= 0 && x < r) || !(y >= 0 && y < r) || !(z >= 0 && z < r)) {
            std::cout << "out of bounds Index" << std::endl;
            assert(false && "out of bounds Index");
        }

        return x + y * resolution + z * resolution * resolution;
    }
};


/*

EdgeIntersection& SampleEdge( EdgeCache& cache, const ivec3& p, int axis, BuildContext_CK& ctx ) {
    uint32_t idx = cache.Index(p.x, p.y, p.z);

    EdgeIntersection* edge;

    if      (axis == 0) edge = &cache.edgesX[idx];
    else if (axis == 1) edge = &cache.edgesY[idx];
    else                edge = &cache.edgesZ[idx];

    if (edge->valid)
        return *edge;

    // -------- COMPUTE --------
    // Zero Crossing Surface
    ivec3 p1 = p;
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

    return *edge;
}


            EdgeIntersection& edgeData = SampleEdge(ctx.edgeCache, leaf->min + CHILD_MIN_OFFSETS[c1], GetEdgeAxis(i++), ctx);
            vec3 p = edgeData.position;		vec3 n = edgeData.normal;
*/





/*


EdgeIntersection& SampleEdge( EdgeCache& cache, const ivec3& p, int axis, BuildContext_CK& ctx ) {
    uint32_t idx = cache.Index(p.x, p.y, p.z);

    EdgeIntersection* edge;

    if      (axis == 0) edge = &cache.edgesX[idx];
    else if (axis == 1) edge = &cache.edgesY[idx];
    else                edge = &cache.edgesZ[idx];

    if (edge->valid)
        return *edge;

    // -------- COMPUTE --------
    // Zero Crossing Surface
    ivec3 p1 = p;
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

    return *edge;
}



		// zero crossing on this edge
		if ((d1 < 0.0f) != (d2 < 0.0f)) {
			 
			// dvec3 p = FindSurfaceEdgeIntersection_ZeroCrossing(c1, c2, d1, d2, leaf->size);
			// p = glm::vec3(leaf->min) + p;
			// const vec3 n = CalculateSurfaceNormal(p); 

			int				axis = 0; 
			if (axis <= 7)	axis = 1;
			else			axis = 2;

			EdgeIntersection& edgeData = SampleEdge(ctx.edgeCache, leaf->min + CHILD_MIN_OFFSETS[c1], axis, ctx); 
			vec3 p = edgeData.position;		vec3 n = edgeData.normal;

			qef.add(p.x, p.y, p.z, n.x, n.y, n.z);
			averageNormal += n;		edgeCount++;
		}






        int GetEdgeAxis(int c1, int c2)
        {
            ivec3 p1 = CHILD_MIN_OFFSETS[c1];
            ivec3 p2 = CHILD_MIN_OFFSETS[c2];

            ivec3 d = p2 - p1;

            if (d.x != 0) return 0;
            if (d.y != 0) return 1;
            return 2;
        }


        int axis = GetEdgeAxis(c1, c2);

        ivec3 p1 = CHILD_MIN_OFFSETS[c1];
        ivec3 p2 = CHILD_MIN_OFFSETS[c2];

        ivec3 base = glm::min(p1, p2);
        ivec3 edgePos = leaf->min + base * leaf->size;

        EdgeIntersection& edgeData =
            SampleEdge(ctx.edgeCache, edgePos, axis, ctx);

        vec3 p = edgeData.position;
        vec3 n = edgeData.normal;


*/








//struct EdgeIntersection
//{
//    bool valid = false;
//    vec3 position;
//    vec3 normal;
//};
//
//struct EdgeCache
//{
//    int rx, ry, rz;
//
//    std::vector<EdgeIntersection> edgesX;
//    std::vector<EdgeIntersection> edgesY;
//    std::vector<EdgeIntersection> edgesZ;
//
//    int IndexX(int x, int y, int z) const
//    {
//        return x + (rx) * (y + (ry)*z);
//    }
//
//    int IndexY(int x, int y, int z) const
//    {
//        return x + (rx) * (y + (ry + 1) * z);
//    }
//
//    int IndexZ(int x, int y, int z) const
//    {
//        return x + (rx) * (y + (ry)*z);
//    }
//};
#include "SurfaceNets.h"
#include "../Graphics/Mesh.h"
#include <cmath>
#include <cassert>
#include <array>
#include <glm/gtx/norm.hpp> // glm::distance2

using glm::vec3;

// ================================================================
// Helpers básicos
// ================================================================

// Indexação 3D → 1D (linear) // helper linearize function - assumes row-major x + y*nx + z*nx*ny
inline size_t linearize3(const size_t cellDimension, uint32_t x, uint32_t y, uint32_t z) { 
    return size_t(x) + size_t(y) * cellDimension + size_t(z) * cellDimension * cellDimension;    //x + y * sizeX + z * sizeX * sizeY;
}

inline size_t linearize3(const size_t cellDimension, glm::vec3 d) {
    return size_t(d.x) + size_t(d.y) * cellDimension + size_t(d.z) * cellDimension * cellDimension;    //x + y * sizeX + z * sizeX * sizeY;
}

// cube constants
static const glm::vec3 CUBE_CORNERS[8] = {
    {0,0,0}, {1,0,0}, {0,1,0}, {1,1,0},
    {0,0,1}, {1,0,1}, {0,1,1}, {1,1,1}
}; 
 
// iterate edges of cube (12)
// edges defined by pairs of corner indices
static const int CUBE_EDGES[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
}; 

static void maybe_make_quad(
    const std::vector<float>& sdf,
    SurfaceNetsBuffer& out,
    size_t p1,
    size_t p2,
    size_t axis_b_stride,
    size_t axis_c_stride)
{
    float d1 = sdf[p1];
    float d2 = sdf[p2];
    bool neg1 = d1 < 0.0f;
    bool neg2 = d2 < 0.0f;
    bool negative_face;
    if (neg1 && !neg2) negative_face = false;
    else if (!neg1 && neg2) negative_face = true;
    else return; // no face

    uint32_t v1 = out.stride_to_index[p1];
    uint32_t v2 = out.stride_to_index[p1 - axis_b_stride];
    uint32_t v3 = out.stride_to_index[p1 - axis_c_stride];
    uint32_t v4 = out.stride_to_index[p1 - axis_b_stride - axis_c_stride];

    if (v1 == NULL_VERTEX || v2 == NULL_VERTEX || v3 == NULL_VERTEX || v4 == NULL_VERTEX)
        return;

    vec3 pos1 = vec3(out.positions[v1].x, out.positions[v1].y, out.positions[v1].z);
    vec3 pos2 = vec3(out.positions[v2].x, out.positions[v2].y, out.positions[v2].z);
    vec3 pos3 = vec3(out.positions[v3].x, out.positions[v3].y, out.positions[v3].z);
    vec3 pos4 = vec3(out.positions[v4].x, out.positions[v4].y, out.positions[v4].z);
     
    std::array<uint32_t, 6> quad;
    if (glm::distance2(pos1, pos4) < glm::distance2(pos2, pos3)) {
        if (negative_face)  quad = { v1,v4,v2, v1,v3,v4 }; 
        else                quad = { v1,v2,v4, v1,v4,v3 }; 
    }
    else {
        if (negative_face)  quad = { v2,v3,v4, v2,v1,v3 }; 
        else                quad = { v2,v4,v3, v2,v3,v1 }; 
    }
    out.indices.insert(out.indices.end(), quad.begin(), quad.end());
} 

static void make_all_quads(const std::vector<float>& sdf, const unsigned int dimension, SurfaceNetsBuffer& out) {
    size_t strideX = linearize3(dimension, 1, 0, 0);        //size_t strideX = 1;
    size_t strideY = linearize3(dimension, 0, 1, 0);        //size_t strideY = dimension;
    size_t strideZ = linearize3(dimension, 0, 0, 1);        //size_t strideZ = dimension * dimension;

    // build quads/triangles between neighboring crossing cells (mirror of make_all_quads)
    for (size_t i = 0; i < out.surface_points.size(); ++i) {
        auto p = out.surface_points[i];
        size_t p_stride = out.surface_strides[i]; 

        // edges parallel X
        if (p.y > 0 && p.z > 0 && p.x < dimension - 1) {
            maybe_make_quad(sdf, out, p_stride, p_stride + strideX, strideY, strideZ);
        }
        // edges parallel Y
        if (p.x > 0 && p.z > 0 && p.y < dimension - 1) {
            maybe_make_quad(sdf, out, p_stride, p_stride + strideY, strideZ, strideX);
        }
        // edges parallel Z
        if (p.x > 0 && p.y > 0 && p.z < dimension - 1) {
            maybe_make_quad(sdf, out, p_stride, p_stride + strideZ, strideX, strideY);
        }
    }
}

// substitui a função SDFGradient antiga
static vec3 SDFGradient(const float* dists, const vec3& s, float voxelSize = 1.0f) {
    // s é a posição local dentro do cube [0..1]^3
    // dists[0..7] são os valores em cada canto com layout:
    // index bit order: (x bit=1 -> corner index bit 0), (y bit -> bit1), (z bit -> bit2)
    // corners: 0:(0,0,0) 1:(1,0,0) 2:(0,1,0) 3:(1,1,0) 4:(0,0,1) 5:(1,0,1) 6:(0,1,1) 7:(1,1,1)

    const float sx = s.x, sy = s.y, sz = s.z;
    const float nx = 1.0f - sx, ny = 1.0f - sy, nz = 1.0f - sz;

    // compute partial derivatives of trilinear interpolation analytically:
    // df/dx = sum_i dists[i] * d(w_i)/dx
    // where w_i = (bitx?sx:nx)*(bity?sy:ny)*(bitz?sz:nz)
    float dfdx = 0.0f;
    float dfdy = 0.0f;
    float dfdz = 0.0f;

    for (int i = 0; i < 8; ++i) {
        //indexando cantos do cubo
        int bx = (i & 1) ? 1 : 0;           //int bitx = i & 1;
        int by = (i & 2) ? 1 : 0;           //int bity = (i >> 1) & 1;
        int bz = (i & 4) ? 1 : 0;           //int bitz = (i >> 2) & 1; 
         
        // weight for yz part (product of y and z factors) without x factor
        float wy = by ? sy : ny;
        float wz = bz ? sz : nz;
        float wx = bx ? sx : nx;

        // derivative of w_i w.r.t x: dw/dx = (bx ? +1 : -1) * wy * wz
        float dw_dx = (bx ? 1.0f : -1.0f) * wy * wz;
        // derivative wrt y
        float dw_dy = (by ? 1.0f : -1.0f) * wx * wz;
        // derivative wrt z
        float dw_dz = (bz ? 1.0f : -1.0f) * wx * wy;

        float v = dists[i];
        dfdx += v * dw_dx;
        dfdy += v * dw_dy;
        dfdz += v * dw_dz;
    }

    // dfdx/dfdy/dfdz are derivatives of the trilinear interpolant with respect to s.x/s.y/s.z
    // if voxelSize != 1, chain rule: ∂/∂world_x = (1/voxelSize) * ∂/∂s.x
    const float invH = (voxelSize != 0.0f) ? (1.0f / voxelSize) : 1.0f;
    vec3 grad = vec3(dfdx * invH, dfdy * invH, dfdz * invH);

    // normalize safely
    float len2 = glm::dot(grad, grad);
    if (len2 <= 1e-12f) {
        // fallback: try a small finite-difference approx directly on corners (central-ish)
        // simple fallback: approximate by axis differences (less accurate but safe)
        float gx = ((dists[1] + dists[3] + dists[5] + dists[7]) - (dists[0] + dists[2] + dists[4] + dists[6])) * 0.5f;
        float gy = ((dists[2] + dists[3] + dists[6] + dists[7]) - (dists[0] + dists[1] + dists[4] + dists[5])) * 0.5f;
        float gz = ((dists[4] + dists[5] + dists[6] + dists[7]) - (dists[0] + dists[1] + dists[2] + dists[3])) * 0.5f;
        grad = vec3(gx * invH, gy * invH, gz * invH);
        len2 = glm::dot(grad, grad);
        if (len2 <= 1e-12f) {
            // last resort: return up vector (avoid NaNs)
            return vec3(0.0f, 1.0f, 0.0f);
        }
    }

    return glm::normalize(grad);
}

// y = y0​ * (1−t) + y1 ​* t =>         Interpolation basic        being T that interpolates where it will be on the edge
static vec3 estimate_surface_edge_intersection(uint32_t c1, uint32_t c2, float v1, float v2) {
    float interp1 = v1 / (v1 - v2);         // t            ==   Acha onde cruza a superfície em 0
    float interp2 = 1.0f - interp1;         // 1- t
    return interp2 * CUBE_CORNERS[c1] + interp1 * CUBE_CORNERS[c2];
}

static vec3 centroid_of_edge_intersections(const float* cornerSDF) {
    int count = 0;
    vec3 sum(0.0f);
    for (auto& edge : CUBE_EDGES) {
        uint32_t a = edge[0], b = edge[1];
        float d1 = cornerSDF[a], d2 = cornerSDF[b];
        if ((d1 < 0.0f) != (d2 < 0.0f)) {
            ++count;
            sum += estimate_surface_edge_intersection(a, b, d1, d2);
        }
    }
    return sum / float(count);
} 

// ===================== ESTIMAR SUPERFÍCIE =====================
static bool estimate_surface_in_cube(SurfaceNetsBuffer& out, const std::vector<float>& sdf, const int cellDimension,
    uint32_t x, uint32_t y, uint32_t z )  { 

    glm::vec3 p = glm::vec3(float(x), float(y), float(z));

    float cornerVal[8]; int num_negative = 0; 
    // collect corner distances
    for (int i = 0; i < 8; ++i) {
        auto off = CUBE_CORNERS[i]; 
         
        if (sdf.size() <= linearize3(cellDimension, p + off)) std::cout << "deu merda " << linearize3(cellDimension, p + off) << "   " << sdf.size();
         
        float d = sdf[linearize3(cellDimension, p + off)];
        cornerVal[i] = d;
        if (d < 0.0f) ++num_negative;
    }
    if (num_negative == 0 || num_negative == 8) return false;     //void block
     
    vec3 c = centroid_of_edge_intersections(cornerVal);
    vec3 normal = SDFGradient(cornerVal, c, /*voxelSize=*/1.0f); // ou params.voxelSize se tiver

    out.positions.push_back({ p.x + c.x, p.y + c.y, p.z + c.z, 1 });       //out.positions.push_back({ p.x + c.x, p.y + c.y, p.z + c.z });
    out.normals.push_back  ({ normal.x, normal.y, normal.z, 1 });          //out.normals.push_back({ normal.x, normal.y, normal.z });




    //----------------EXTRA     -----------DEBUG
    // --- debug: push cube corners (world coords) ---
    // opção A: todos os 8 cantos
    for (int i = 0; i < 8; ++i) { out.debug_corners.insert(p + CUBE_CORNERS[i]); }

    /*for (int i = 0; i < 8; ++i) {
        glm::vec3 cornerFloat = p + CUBE_CORNERS[i];           // float
        out.debug_corners.insert( glm::ivec3(glm::round(cornerFloat)) ); }  // converte para inteiro
     */

    // opção B (apenas cantos com densidade negativa)
    //for (int i = 0; i < 8; ++i) if (cornerVal[i] < 0.0f) out.debug_corners.emplace_back(p + CUBE_CORNERS[i]);

    return true;
}

static void estimate_surface(const std::vector<float>& sdf, const int cellDimension, SurfaceNetsBuffer& out ) {
    const int cellsX = cellDimension - 1; const int cellsY = cellDimension - 1; const int cellsZ = cellDimension - 1; 
    // For each cell
    for (uint32_t z = 0; z < cellsZ; ++z) for (uint32_t y = 0; y < cellsY; ++y) for (uint32_t x = 0; x < cellsX; ++x) { 
        size_t stride = linearize3(cellDimension, x, y, z);
        if (estimate_surface_in_cube(out, sdf, cellDimension, x, y, z)) {
            if (out.stride_to_index.size() <= stride) {
                std::cout << "deu merda " << stride << "   " << out.stride_to_index.size();
            }
            out.stride_to_index[stride] = uint32_t(out.positions.size() - 1);
            out.surface_points.push_back(glm::vec3(x, y, z));   //coordenadas do voxel
            out.surface_strides.push_back(uint32_t(stride));
        } else {
            out.stride_to_index[stride] = NULL_VERTEX;
        } 
    }

}

Mesh* SurfaceNetsCPU::Generate(const VoxelGrid& sdf, const SurfaceNetsParams& params, SurfaceNetsBuffer& out) {
    size_t arraySize = size_t(params.cellDimension) * params.cellDimension * params.cellDimension;
    assert(arraySize == sdf.density.size());
    out.reset(arraySize);
     
    estimate_surface(sdf.density, params.cellDimension, out); 
    make_all_quads  (sdf.density, params.cellDimension, out); 
      
    // Criar Mesh diretamente de posições/normais/índices
    Mesh* mesh = new Mesh();
    mesh->UploadFromRaw(out.positions, out.normals, out.indices);
    std::cout << "Vertices: 4 \n ";
    return mesh;
}



// opcional: montar um Mesh real usando out.positions/out.normals/out.indices
/*Mesh* mesh = new Mesh();
mesh->vertices = out.positions;
mesh->normals = out.normals;
mesh->indices = out.indices;
mesh->UploadToGPU();
return mesh;*/


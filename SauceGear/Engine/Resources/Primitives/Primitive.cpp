#include "Primitive.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

// =======================
// CACHE
// =======================

std::shared_ptr<MeshAsset> PrimitiveMesh::cube = nullptr;
std::shared_ptr<MeshAsset> PrimitiveMesh::cubeInverse = nullptr;
std::shared_ptr<MeshAsset> PrimitiveMesh::sphere = nullptr;
std::shared_ptr<MeshAsset> PrimitiveMesh::plane = nullptr;
std::shared_ptr<MeshAsset> PrimitiveMesh::cylinder = nullptr;

// =======================
// PUBLIC API
// =======================

std::shared_ptr<MeshAsset> PrimitiveMesh::Cube() {
    if (cube) return cube;

    std::vector<Vertex> v;
    std::vector<uint32_t> i;
    BuildCube(v, i);

    cube = std::make_shared<MeshAsset>();
    cube->name = "Primitive_Cube"; 
    return cube;
}

std::shared_ptr<MeshAsset> PrimitiveMesh::CubeInverse() {
    if (cubeInverse) return cubeInverse;

    std::vector<Vertex> v;
    std::vector<uint32_t> i;
    BuildCubeInverse(v, i);

    cubeInverse = std::make_shared<MeshAsset>();
    cubeInverse->name = "Primitive_Cube_Inverse"; 
    return cubeInverse;
}

std::shared_ptr<MeshAsset> PrimitiveMesh::Plane() {
    if (plane) return plane;

    std::vector<Vertex> v;
    std::vector<uint32_t> i;
    BuildPlane(v, i);

    plane = std::make_shared<MeshAsset>();
    plane->name = "Primitive_Plane"; 
    return plane;
}

std::shared_ptr<MeshAsset> PrimitiveMesh::Sphere(
    unsigned segments,
    unsigned rings,
    float radius
) {
    std::vector<Vertex> v;
    std::vector<uint32_t> i;
    BuildSphere(v, i, segments, rings, radius);

    auto mesh = std::make_shared<MeshAsset>();
    mesh->name = "Primitive_Sphere"; 
    return mesh;
}

std::shared_ptr<MeshAsset> PrimitiveMesh::Cylinder(
    unsigned segments,
    float height,
    float radius,
    bool capped
) {
    std::vector<Vertex> v;
    std::vector<uint32_t> i;
    BuildCylinder(v, i, segments, height, radius, capped);

    auto mesh = std::make_shared<MeshAsset>();
    mesh->name = "Primitive_Cylinder"; 
    return mesh;
}

// =======================
// BUILDERS
// =======================

static void AddQuad(
    std::vector<Vertex>& v,
    std::vector<uint32_t>& i,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c,
    const glm::vec3& d,
    const glm::vec3& n
) {
    uint32_t start = (uint32_t)v.size();

    v.push_back({ a, n, {0,0} });
    v.push_back({ b, n, {1,0} });
    v.push_back({ c, n, {1,1} });
    v.push_back({ d, n, {0,1} });

    // CCW
    i.insert(i.end(), {
        start, start + 1, start + 2,
        start, start + 2, start + 3
        });
}

// ---------- CUBE ----------

void PrimitiveMesh::BuildCube(
    std::vector<Vertex>& v,
    std::vector<uint32_t>& i
) {
    float h = 0.5f;

    AddQuad(v, i, { -h,-h, h }, { h,-h, h }, { h, h, h }, { -h, h, h }, { 0,0,1 });   // front
    AddQuad(v, i, { h,-h,-h }, { -h,-h,-h }, { -h, h,-h }, { h, h,-h }, { 0,0,-1 }); // back
    AddQuad(v, i, { -h, h, h }, { h, h, h }, { h, h,-h }, { -h, h,-h }, { 0,1,0 });  // top
    AddQuad(v, i, { -h,-h,-h }, { h,-h,-h }, { h,-h, h }, { -h,-h, h }, { 0,-1,0 }); // bottom
    AddQuad(v, i, { h,-h, h }, { h,-h,-h }, { h, h,-h }, { h, h, h }, { 1,0,0 });    // right
    AddQuad(v, i, { -h,-h,-h }, { -h,-h, h }, { -h, h, h }, { -h, h,-h }, { -1,0,0 });// left
}

// ---------- INVERSE CUBE ----------

void PrimitiveMesh::BuildCubeInverse(
    std::vector<Vertex>& v,
    std::vector<uint32_t>& i
) {
    BuildCube(v, i);
    for (auto& idx : i) std::swap(i[idx], i[idx]); // winding invertido pelo culling
}

// ---------- PLANE ----------

void PrimitiveMesh::BuildPlane(
    std::vector<Vertex>& v,
    std::vector<uint32_t>& i
) {
    float h = 0.5f;

    AddQuad(
        v, i,
        { -h,0,-h }, { h,0,-h }, { h,0, h }, { -h,0, h },
        { 0,1,0 }
    );
}

// ---------- SPHERE ----------

void PrimitiveMesh::BuildSphere(
    std::vector<Vertex>& v,
    std::vector<uint32_t>& i,
    unsigned segments,
    unsigned rings,
    float radius
) {
    for (unsigned y = 0; y <= rings; ++y) {
        float vPos = (float)y / rings;
        float phi = vPos * glm::pi<float>();

        for (unsigned x = 0; x <= segments; ++x) {
            float uPos = (float)x / segments;
            float theta = uPos * glm::two_pi<float>();

            glm::vec3 p{
                std::sin(phi) * std::cos(theta),
                std::cos(phi),
                std::sin(phi) * std::sin(theta)
            };

            v.push_back({ p * radius, glm::normalize(p), {uPos, vPos} });
        }
    }

    for (unsigned y = 0; y < rings; ++y) {
        for (unsigned x = 0; x < segments; ++x) {
            uint32_t a = y * (segments + 1) + x;
            uint32_t b = a + segments + 1;

            i.insert(i.end(), {
                a, b, a + 1,
                a + 1, b, b + 1
                });
        }
    }
}

// ---------- CYLINDER ----------

void PrimitiveMesh::BuildCylinder(
    std::vector<Vertex>& v,
    std::vector<uint32_t>& i,
    unsigned segments,
    float height,
    float radius,
    bool capped
) {
    float h = height * 0.5f;

    // SIDES
    for (unsigned s = 0; s <= segments; ++s) {
        float t = (float)s / segments;
        float a = t * glm::two_pi<float>();

        float x = std::cos(a);
        float z = std::sin(a);

        glm::vec3 n = { x,0,z };

        v.push_back({ { x * radius, -h, z * radius }, n, { t,0 } });
        v.push_back({ { x * radius,  h, z * radius }, n, { t,1 } });
    }

    for (unsigned s = 0; s < segments; ++s) {
        uint32_t i0 = s * 2;
        uint32_t i1 = i0 + 1;
        uint32_t i2 = i0 + 2;
        uint32_t i3 = i0 + 3;

        i.insert(i.end(), {
            i0, i2, i1,
            i1, i2, i3
            });
    }

    if (!capped) return;

    // TOP
    uint32_t topCenter = (uint32_t)v.size();
    v.push_back({ {0,h,0}, {0,1,0}, {0.5f,0.5f} });

    for (unsigned s = 0; s <= segments; ++s) {
        float t = (float)s / segments;
        float a = t * glm::two_pi<float>();
        v.push_back({ { std::cos(a) * radius, h, std::sin(a) * radius }, {0,1,0}, {0,0} });
    }

    for (unsigned s = 0; s < segments; ++s)
        i.insert(i.end(), { topCenter, topCenter + s + 1, topCenter + s + 2 });

    // BOTTOM
    uint32_t botCenter = (uint32_t)v.size();
    v.push_back({ {0,-h,0}, {0,-1,0}, {0.5f,0.5f} });

    for (unsigned s = 0; s <= segments; ++s) {
        float t = (float)s / segments;
        float a = t * glm::two_pi<float>();
        v.push_back({ { std::cos(a) * radius, -h, std::sin(a) * radius }, {0,-1,0}, {0,0} });
    }

    for (unsigned s = 0; s < segments; ++s)
        i.insert(i.end(), { botCenter, botCenter + s + 2, botCenter + s + 1 });
}

std::shared_ptr<MeshAsset>
PrimitiveMesh::CreateSphere2RenderingLight(uint32_t xSegments, uint32_t ySegments) { 
    static std::shared_ptr<MeshAsset> sphereLightAsset;
    if (sphereLightAsset) return sphereLightAsset;      //cached

    //Build
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (uint32_t y = 0; y <= ySegments; ++y) {
        for (uint32_t x = 0; x <= xSegments; ++x) {
            float xSegment = (float)x / xSegments;
            float ySegment = (float)y / ySegments;

            float xPos = std::cos(xSegment * 2.0f * glm::pi<float>()) *
                std::sin(ySegment * glm::pi<float>());
            float yPos = std::cos(ySegment * glm::pi<float>());
            float zPos = std::sin(xSegment * 2.0f * glm::pi<float>()) *
                std::sin(ySegment * glm::pi<float>());

            Vertex v{};
            v.Position = glm::vec3(xPos, yPos, zPos);
            v.Normal = glm::normalize(v.Position);
            v.TexCoords = { xSegment, ySegment };

            vertices.push_back(v);
        }
    }

    for (uint32_t y = 0; y < ySegments; ++y) {
        for (uint32_t x = 0; x < xSegments; ++x) {
            uint32_t i0 = y * (xSegments + 1) + x;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + (xSegments + 1);
            uint32_t i3 = i2 + 1;

            indices.insert(indices.end(), {
                i0, i2, i1,
                i1, i2, i3
                });
        }
    }

    sphereLightAsset = std::make_shared<MeshAsset>();
    sphereLightAsset->name = "Sphere_Light";
    sphereLightAsset->SetData(std::move(vertices), std::move(indices));

    return sphereLightAsset;
}


std::shared_ptr<MeshAsset>
PrimitiveMesh::CreateTorus(
    uint32_t segments,
    uint32_t ringSegments,
    float majorRadius,
    float minorRadius) 
{

    static std::shared_ptr<MeshAsset> torusAsset;
    if (torusAsset)
        return torusAsset;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i <= segments; ++i) {
        float u = float(i) / segments;
        float theta = u * 2.0f * glm::pi<float>();

        glm::vec3 circleCenter = {
            std::cos(theta) * majorRadius,
            0.0f,
            std::sin(theta) * majorRadius
        };

        for (uint32_t j = 0; j <= ringSegments; ++j) {
            float v = float(j) / ringSegments;
            float phi = v * 2.0f * glm::pi<float>();

            glm::vec3 normal = {
                std::cos(theta) * std::cos(phi),
                std::sin(phi),
                std::sin(theta) * std::cos(phi)
            };

            Vertex vert{};
            vert.Position = circleCenter + normal * minorRadius;
            vert.Normal = glm::normalize(normal);
            vert.TexCoords = { u, v };

            vertices.push_back(vert);
        }
    }

    for (uint32_t i = 0; i < segments; ++i) {
        for (uint32_t j = 0; j < ringSegments; ++j) {
            uint32_t a = i * (ringSegments + 1) + j;
            uint32_t b = a + ringSegments + 1;

            indices.insert(indices.end(), {
                a, b, a + 1,
                a + 1, b, b + 1
                });
        }
    }

    torusAsset = std::make_shared<MeshAsset>();
    torusAsset->name = "Torus";
    torusAsset->SetData(std::move(vertices), std::move(indices));

    return torusAsset;
}

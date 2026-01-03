#pragma once
#include <glm/glm.hpp>
#include <cfloat>
#include "../Math/AABB.h"  
#include "../Graphics/Mesh.h" 

#include "../Core/EngineContext.h"
#include "../Scene/SceneECS.h" 
#include "../ECS/Components/TransformComponent.h"
#include "../ECS/Components/MeshRenderer.h"

class AABBBuilder {
public:   
    static inline void aabbMesh(Entity e) {
        if (!(scn->HasComponent<MeshRenderer>(e) && scn->HasComponent<TransformComponent>(e))) return; 
        auto& aabb = scn->AddComponent<AABBComponent>(e);

        if (scn->GetComponent<MeshRenderer>(e).mesh == nullptr) std::cout << "errado aq" << std::endl;
        auto local = ComputeLocalAABB(*scn->GetComponent<MeshRenderer>(e).mesh);
        aabb.setLocal(local.min, local.max);

        //auto world = TransformAABB(local, scn->GetComponent<TransformComponent>(e).model);
        //aabb.setWorld(world.min, world.max);

        // 2️⃣ World AABB NÃO aqui
        aabb.dirty = true;
    }

    static inline AABB aabbMesh(Mesh& mesh, glm::mat4 m) {
        return TransformAABB(ComputeLocalAABB(mesh), m);
    }

    static inline AABB ComputeLocalAABB(const Mesh& mesh) {
        AABB box;
        for (const auto& v : mesh.vertices) {
            box.min = glm::min(box.min, v.Position);
            box.max = glm::max(box.max, v.Position);
        }
        return box;
    }

    static inline AABB TransformAABB(const AABB& local, const glm::mat4& m) {
        glm::vec3 center = local.center();
        glm::vec3 extents = local.size() * 0.5f;

        glm::vec3 newCenter = glm::vec3(m * glm::vec4(center, 1.0f));

        glm::mat3 absMat = glm::mat3(
            glm::abs(glm::vec3(m[0])),
            glm::abs(glm::vec3(m[1])),
            glm::abs(glm::vec3(m[2]))
        );

        glm::vec3 newExtents = absMat * extents;

        return AABB(newCenter - newExtents, newCenter + newExtents);
    }








    static AABB FromMesh(
        const Mesh& mesh,
        const glm::mat4& transform)
    {
        AABB box;

        for (const auto& v : mesh.vertices) {
            glm::vec3 p = glm::vec3(transform * glm::vec4(v.Position, 1.0f));
            box.min = glm::min(box.min, p);
            box.max = glm::max(box.max, p);
        }
        return box;
    }
     

    /*void CalculateAABB(std::vector<Vertex> vertices, const glm::mat4& transform) {
        min = glm::vec3(FLT_MAX);
        max = glm::vec3(-FLT_MAX);

        for (const auto& v : vertices) {
            glm::vec3 pos = glm::vec3(transform * glm::vec4(v.Position, 1.0f));
            min = glm::min(min, pos);
            max = glm::max(max, pos);
        }
    }*/
};

 









//static BoundingSphereComponent CalculateBoundingSphere(Mesh* mesh, const glm::mat4& transform) {
    //    BoundingSphereComponent sphere;
    //    glm::vec3 centerSum(0.0f);
    //    int vertexCount = mesh->vertices.size();

    //    // Calcula centro médio
    //    for (const auto& v : mesh->vertices) {
    //        glm::vec3 pos = glm::vec3(transform * glm::vec4(v.Position, 1.0f));
    //        centerSum += pos;
    //    }
    //    sphere.center = centerSum / (float)vertexCount;

    //    // Calcula raio máximo
    //    float maxDist2 = 0.0f;
    //    for (const auto& v : mesh->vertices) {
    //        glm::vec3 pos = glm::vec3(transform * glm::vec4(v.Position, 1.0f));
    //        float dist2 = glm::length2(pos - sphere.center);
    //        maxDist2 = std::max(maxDist2, dist2);
    //    }
    //    sphere.radius = sqrt(maxDist2);

    //    return sphere;
    //}
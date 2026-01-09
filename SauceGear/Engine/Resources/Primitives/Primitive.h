#pragma once
#include <memory>
#include <unordered_map>
#include "../../Assets/MeshAsset.h"

class PrimitiveMesh {
public:
    static std::shared_ptr<MeshAsset> Cube();
    static std::shared_ptr<MeshAsset> CubeInverse();
    static std::shared_ptr<MeshAsset> Sphere(
        unsigned int segments = 32,
        unsigned int rings = 16,
        float radius = 1.0f
    );
    static std::shared_ptr<MeshAsset> Plane();
    static std::shared_ptr<MeshAsset> Cylinder(
        unsigned int segments = 32,
        float height = 1.0f,
        float radius = 0.5f,
        bool capped = true
    );

    static std::shared_ptr<MeshAsset>
        CreateSphere2RenderingLight(uint32_t xSegments = 32, uint32_t ySegments = 32);

    static std::shared_ptr<MeshAsset>
        CreateTorus(uint32_t segments = 32,
            uint32_t ringSegments = 24,
            float majorRadius = 1.0f,
            float minorRadius = 0.25f);


private:
    static std::shared_ptr<MeshAsset> cube;
    static std::shared_ptr<MeshAsset> cubeInverse;
    static std::shared_ptr<MeshAsset> sphere;
    static std::shared_ptr<MeshAsset> plane;
    static std::shared_ptr<MeshAsset> cylinder;

    // builders (PUROS)
    static void BuildCube(std::vector<Vertex>& v, std::vector<uint32_t>& i);
    static void BuildCubeInverse(std::vector<Vertex>& v, std::vector<uint32_t>& i);
    static void BuildSphere(
        std::vector<Vertex>& v,
        std::vector<uint32_t>& i,
        unsigned int segments,
        unsigned int rings,
        float radius
    );
    static void BuildPlane(std::vector<Vertex>& v, std::vector<uint32_t>& i);
    static void BuildCylinder(
        std::vector<Vertex>& v,
        std::vector<uint32_t>& i,
        unsigned int segments,
        float height,
        float radius,
        bool capped
    );
};

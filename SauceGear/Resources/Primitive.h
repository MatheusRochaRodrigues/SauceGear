#pragma once
#include "../Graphics/Mesh.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class PrimitiveMesh {
public:
    static Mesh* CreateCube(shared_ptr<MaterialInstance> material = nullptr);
    static Mesh* CreateInverseCube(shared_ptr<MaterialInstance> material = nullptr);
    static Mesh* CreateSphere(shared_ptr<MaterialInstance> material = nullptr, unsigned int segments = 32, unsigned int rings = 16, float radius = 1.0f);
    static Mesh* CreatePlane(shared_ptr<MaterialInstance> material = nullptr);
    static Mesh* CreateCylinder(shared_ptr<MaterialInstance> material = nullptr, unsigned int segments = 32, float height = 1.0f, float radius = 0.5f, bool capped = true);

    static Mesh* CreateSphere2RenderingLight(unsigned int xSegments = 32, unsigned int ySegments = 32);
private:
    static std::unique_ptr<Mesh> sphereMeshLight;

    static std::unique_ptr<Mesh> cubeMesh;
    static std::unique_ptr<Mesh> cubeInverseMesh;
    static std::unique_ptr<Mesh> planeMesh;
    static std::unique_ptr<Mesh> sphereMesh;
    static std::unique_ptr<Mesh> cylinderMesh;

    static Mesh Cube(shared_ptr<MaterialInstance> material = nullptr);
    static Mesh CubeInverse(shared_ptr<MaterialInstance> material = nullptr);
    static Mesh Sphere(unsigned int segments = 32, unsigned int rings = 16, float radius = 1.0f, shared_ptr<MaterialInstance> material = nullptr);
    static Mesh Cylinder(unsigned int segments = 32, float height = 1.0f, float radius = 0.5f, bool capped = true, shared_ptr<MaterialInstance> material = nullptr);

    static const Mesh& CreateTorus(unsigned int segments = 32, unsigned int ringSegments = 24, float majorRadius = 1.0f, float minorRadius = 0.25f);

};


//Mesh* CreateCubeMesh() {
//    Mesh cubeMesh;
//    cubeMesh.vertices = {
//        // position          normal           texcoords
//        -1, -1, -1,  0,  0, -1,  0, 0,
//         1,  1, -1,  0,  0, -1,  1, 1,
//         1, -1, -1,  0,  0, -1,  1, 0,
//         1,  1, -1,  0,  0, -1,  1, 1,
//        -1, -1, -1,  0,  0, -1,  0, 0,
//        -1,  1, -1,  0,  0, -1,  0, 1,
//        // ... restantes omitidos por brevidade (voc� pode usar o `renderCube()` original)
//    };
//
//    cubeMesh.SetupMesh();
//
//    auto* cubeMesh = new Mesh();
//    cubeMesh->AddMesh(cubeMesh);
//
//    return cubeMesh;
//}



//Entity e = CreateEntity();
//auto& t = AddComponent<Transform>(e);
//auto& renderer = AddComponent<MeshRenderer>(e);
//renderer.Mesh = CreateCubeMesh(); // Em vez de carregar de arquivo
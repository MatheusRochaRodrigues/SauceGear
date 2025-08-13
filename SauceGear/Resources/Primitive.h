#pragma once
#include "Model.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class PrimitiveMesh {
public:
    static Model* CreateCube(Material* material = nullptr);
    static Model* CreateInverseCube(Material* material = nullptr);
    static Model* CreateSphere(Material* material = nullptr, unsigned int segments = 32, unsigned int rings = 16, float radius = 1.0f);
    static Model* CreatePlane(Material* material = nullptr);
    static Model* CreateCylinder(Material* material = nullptr, unsigned int segments = 32, float height = 1.0f, float radius = 0.5f, bool capped = true);

    static Mesh* CreateSphere2RenderingLight(unsigned int xSegments = 32, unsigned int ySegments = 32);
private:
    static std::unique_ptr<Mesh> sphereMeshLight;

    static std::unique_ptr<Model> cubeMesh;
    static std::unique_ptr<Model> cubeInverseMesh;
    static std::unique_ptr<Model> planeMesh;
    static std::unique_ptr<Model> sphereMesh;
    static std::unique_ptr<Model> cylinderMesh;

    static Mesh Cube(Material* material = nullptr);
    static Mesh CubeInverse(Material* material = nullptr);
    static Mesh Sphere(unsigned int segments = 32, unsigned int rings = 16, float radius = 1.0f, Material* material = nullptr);
    static Mesh Cylinder(unsigned int segments = 32, float height = 1.0f, float radius = 0.5f, bool capped = true, Material* material = nullptr);

    static const Mesh& CreateTorus(unsigned int segments = 32, unsigned int ringSegments = 24, float majorRadius = 1.0f, float minorRadius = 0.25f);

};


//Model* CreateCubeModel() {
//    Mesh cubeMesh;
//    cubeMesh.vertices = {
//        // position          normal           texcoords
//        -1, -1, -1,  0,  0, -1,  0, 0,
//         1,  1, -1,  0,  0, -1,  1, 1,
//         1, -1, -1,  0,  0, -1,  1, 0,
//         1,  1, -1,  0,  0, -1,  1, 1,
//        -1, -1, -1,  0,  0, -1,  0, 0,
//        -1,  1, -1,  0,  0, -1,  0, 1,
//        // ... restantes omitidos por brevidade (você pode usar o `renderCube()` original)
//    };
//
//    cubeMesh.SetupMesh();
//
//    auto* cubeModel = new Model();
//    cubeModel->AddMesh(cubeMesh);
//
//    return cubeModel;
//}



//Entity e = CreateEntity();
//auto& t = AddComponent<Transform>(e);
//auto& renderer = AddComponent<MeshRenderer>(e);
//renderer.model = CreateCubeModel(); // Em vez de carregar de arquivo
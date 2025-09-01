#include "Primitive.h"
#include <cmath>

std::unique_ptr<Mesh> PrimitiveMesh::cubeMesh = nullptr;
Mesh* PrimitiveMesh::CreateCube(Material* material) {
    if (!cubeMesh) {
        cubeMesh = std::make_unique<Mesh>(Cube(material)); 
        cubeMesh->name = "Cube";
    }
    return cubeMesh.get();
}

std::unique_ptr<Mesh> PrimitiveMesh::cubeInverseMesh = nullptr;
Mesh* PrimitiveMesh::CreateInverseCube(Material* material) {
    if (!cubeInverseMesh) {
        cubeInverseMesh = std::make_unique<Mesh>(CubeInverse(material)); 
    }
    return cubeInverseMesh.get();
}

std::unique_ptr<Mesh> PrimitiveMesh::sphereMesh = nullptr;
Mesh* PrimitiveMesh::CreateSphere(Material* material, unsigned int segments, unsigned int rings, float radius) {
    if (!sphereMesh) {
        sphereMesh = std::make_unique<Mesh>(Sphere(segments, rings, radius, material));
        sphereMesh->name = "Sphere";
    }
    return sphereMesh.get();
} 

std::unique_ptr<Mesh> PrimitiveMesh::cylinderMesh = nullptr;
Mesh* PrimitiveMesh::CreateCylinder(Material* material, unsigned int segments, float height, float radius, bool capped) {
    if (!cylinderMesh) {
        cylinderMesh = std::make_unique<Mesh>(Cylinder(segments, height, radius, material)); 
    }
    return cylinderMesh.get();
}
 
std::unique_ptr<Mesh> PrimitiveMesh::planeMesh = nullptr;
Mesh* PrimitiveMesh::CreatePlane(Material* material) {
    if (!planeMesh) { 
        std::vector<Vertex> vertices = {
            // Posi��o    // Normal   // UV   // Tangent  // Bitangent  // BoneIDs         // Weights
            { {-1, 0, -1}, {0, 1, 0}, {0, 0}, {1, 0, 0}, {0, 0, 1}, {0,0,0,0}, {0,0,0,0} },
            { { 1, 0, -1}, {0, 1, 0}, {1, 0}, {1, 0, 0}, {0, 0, 1}, {0,0,0,0}, {0,0,0,0} },
            { { 1, 0,  1}, {0, 1, 0}, {1, 1}, {1, 0, 0}, {0, 0, 1}, {0,0,0,0}, {0,0,0,0} },
            { {-1, 0,  1}, {0, 1, 0}, {0, 1}, {1, 0, 0}, {0, 0, 1}, {0,0,0,0}, {0,0,0,0} },
        }; 
        /*std::vector<unsigned int> indices = {
            0, 1, 2, 2, 3, 0
        };*/

        // Ordem anti-hor�ria vista de cima:
        std::vector<unsigned int> indices = {
            0, 3, 2,
            2, 1, 0
        };

        planeMesh = std::make_unique<Mesh>(vertices, indices, material);
        planeMesh->name = "Plane";
    }
    return planeMesh.get();
}


Mesh PrimitiveMesh::CubeInverse(Material* material) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Posi��es �nicas dos cantos do cubo
    glm::vec3 positions[] = {
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, { -1,  1, -1}, // Z-
        {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, { -1,  1,  1}  // Z+
    };

    //std::vector<Vertex> vertices = {
    //    {{-1, -1, -1}, {0,0,0}, {0,0}}, // 0
    //    {{ 1, -1, -1}, {0,0,0}, {1,0}}, // 1
    //    {{ 1,  1, -1}, {0,0,0}, {1,1}}, // 2
    //    {{-1,  1, -1}, {0,0,0}, {0,1}}, // 3
    //    {{-1, -1,  1}, {0,0,0}, {0,0}}, // 4
    //    {{ 1, -1,  1}, {0,0,0}, {1,0}}, // 5
    //    {{ 1,  1,  1}, {0,0,0}, {1,1}}, // 6
    //    {{-1,  1,  1}, {0,0,0}, {0,1}}  // 7
    //};

    // Normais para cada face
    glm::vec3 normals[] = {
        { 0,  0, -1}, // Z-
        { 0,  0,  1}, // Z+
        { 0, -1,  0}, // Y-
        { 0,  1,  0}, // Y+
        {-1,  0,  0}, // X-
        { 1,  0,  0}  // X+
    };

    // UVs padr�o
    glm::vec2 tex[] = {
        {0, 0}, {1, 0}, {1, 1}, {0, 1}
    };

    // Para cada face: 4 v�rtices (em anti-hor�rio), normais, tangente, bitangente
    auto addFace = [&](glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3,
        glm::vec3 normal) {
            glm::vec3 tangent, bitangent;
            {
                // Calcula tangente/bitangente para UV padr�o
                glm::vec3 edge1 = v1 - v0;
                glm::vec3 edge2 = v2 - v0;
                glm::vec2 deltaUV1 = tex[1] - tex[0];
                glm::vec2 deltaUV2 = tex[2] - tex[0];

                float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                tangent = glm::normalize(tangent);

                bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                bitangent = glm::normalize(bitangent);
            }

            unsigned int startIndex = vertices.size();

            Vertex verts[4];
            verts[0] = { v0, normal, tex[0], tangent, bitangent };
            verts[1] = { v1, normal, tex[1], tangent, bitangent };
            verts[2] = { v2, normal, tex[2], tangent, bitangent };
            verts[3] = { v3, normal, tex[3], tangent, bitangent };

            for (int i = 0; i < 4; ++i)
                vertices.push_back(verts[i]);

            // Tri�ngulos em sentido hor�rio
            indices.push_back(startIndex + 0);
            indices.push_back(startIndex + 1);
            indices.push_back(startIndex + 2);

            indices.push_back(startIndex + 0);
            indices.push_back(startIndex + 2);
            indices.push_back(startIndex + 3);
        };
     

    // Faces em anti-hor�rio
    addFace(positions[0], positions[1], positions[2], positions[3], normals[0] * glm::vec3(-1)); // Back (Z-)
    addFace(positions[5], positions[4], positions[7], positions[6], normals[1] * glm::vec3(-1)); // Front (Z+)
    addFace(positions[4], positions[5], positions[1], positions[0], normals[2] * glm::vec3(-1)); // Bottom (Y-)
    addFace(positions[3], positions[2], positions[6], positions[7], normals[3] * glm::vec3(-1)); // Top (Y+)
    addFace(positions[4], positions[0], positions[3], positions[7], normals[4] * glm::vec3(-1)); // Left (X-)
    addFace(positions[1], positions[5], positions[6], positions[2], normals[5] * glm::vec3(-1)); // Right (X+)

    return Mesh(vertices, indices, material);
}

Mesh PrimitiveMesh::Cube(Material* material) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Posi��es �nicas dos cantos do cubo
    glm::vec3 positions[] = {
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, { -1,  1, -1}, // Z-
        {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, { -1,  1,  1}  // Z+
    };

    //std::vector<Vertex> vertices = {
    //    {{-1, -1, -1}, {0,0,0}, {0,0}}, // 0
    //    {{ 1, -1, -1}, {0,0,0}, {1,0}}, // 1
    //    {{ 1,  1, -1}, {0,0,0}, {1,1}}, // 2
    //    {{-1,  1, -1}, {0,0,0}, {0,1}}, // 3
    //    {{-1, -1,  1}, {0,0,0}, {0,0}}, // 4
    //    {{ 1, -1,  1}, {0,0,0}, {1,0}}, // 5
    //    {{ 1,  1,  1}, {0,0,0}, {1,1}}, // 6
    //    {{-1,  1,  1}, {0,0,0}, {0,1}}  // 7
    //};

    // Normais para cada face
    glm::vec3 normals[] = {
        { 0,  0, -1}, // Z-
        { 0,  0,  1}, // Z+
        { 0, -1,  0}, // Y-
        { 0,  1,  0}, // Y+
        {-1,  0,  0}, // X-
        { 1,  0,  0}  // X+
    };

    // UVs padr�o
    glm::vec2 tex[] = {
        {0, 0}, {1, 0}, {1, 1}, {0, 1}
    };

    // Para cada face: 4 v�rtices (em anti-hor�rio), normais, tangente, bitangente
    auto addFace = [&](glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3,
        glm::vec3 normal) {
            glm::vec3 tangent, bitangent;
            {
                // Calcula tangente/bitangente para UV padr�o
                glm::vec3 edge1 = v1 - v0;
                glm::vec3 edge2 = v2 - v0;
                glm::vec2 deltaUV1 = tex[1] - tex[0];
                glm::vec2 deltaUV2 = tex[2] - tex[0];

                float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                tangent = glm::normalize(tangent);

                bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                bitangent = glm::normalize(bitangent);
            }

            unsigned int startIndex = vertices.size();

            Vertex verts[4];
            verts[0] = { v0, normal, tex[0], tangent, bitangent };
            verts[1] = { v1, normal, tex[1], tangent, bitangent };
            verts[2] = { v2, normal, tex[2], tangent, bitangent };
            verts[3] = { v3, normal, tex[3], tangent, bitangent };

            for (int i = 0; i < 4; ++i)
                vertices.push_back(verts[i]);

            // Tri�ngulos em anti-hor�rio
            indices.push_back(startIndex + 0);
            indices.push_back(startIndex + 2);
            indices.push_back(startIndex + 1);

            indices.push_back(startIndex + 0);
            indices.push_back(startIndex + 3);
            indices.push_back(startIndex + 2);
        };

    //// �ndices � anti-hor�rio (CCW)
    //std::vector<unsigned int> indices = {
    //    // Frente
    //    4, 5, 6,
    //    4, 6, 7,
    //    // Tr�s
    //    0, 2, 1,
    //    0, 3, 2,
    //    // Esquerda
    //    0, 7, 3,
    //    0, 4, 7,
    //    // Direita
    //    1, 2, 6,
    //    1, 6, 5,
    //    // Topo
    //    3, 7, 6,
    //    3, 6, 2,
    //    // Base
    //    0, 1, 5,
    //    0, 5, 4
    //};

    // Faces em anti-hor�rio
    addFace(positions[0], positions[1], positions[2], positions[3], normals[0]); // Back (Z-)
    addFace(positions[5], positions[4], positions[7], positions[6], normals[1]); // Front (Z+)
    addFace(positions[4], positions[5], positions[1], positions[0], normals[2]); // Bottom (Y-)
    addFace(positions[3], positions[2], positions[6], positions[7], normals[3]); // Top (Y+)
    addFace(positions[4], positions[0], positions[3], positions[7], normals[4]); // Left (X-)
    addFace(positions[1], positions[5], positions[6], positions[2], normals[5]); // Right (X+)

    return Mesh(vertices, indices, material);
}




Mesh PrimitiveMesh::Sphere(unsigned int segments, unsigned int rings, float radius, Material* material) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int y = 0; y <= rings; ++y) {
        for (unsigned int x = 0; x <= segments; ++x) {
            float xSegment = (float)x / segments;
            float ySegment = (float)y / rings;
            float xPos = radius * std::cos(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);
            float yPos = radius * std::cos(ySegment * M_PI);
            float zPos = radius * std::sin(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);

            Vertex vertex{};
            vertex.Position = glm::vec3(xPos, yPos, zPos);
            vertex.Normal = glm::normalize(vertex.Position);
            vertex.TexCoords = glm::vec2(xSegment, ySegment);
            vertices.push_back(vertex);
        }
    }

    for (unsigned int y = 0; y < rings; ++y) {
        for (unsigned int x = 0; x < segments; ++x) {
            unsigned int i0 = y * (segments + 1) + x;
            unsigned int i1 = i0 + 1;
            unsigned int i2 = i0 + (segments + 1);
            unsigned int i3 = i2 + 1;

            /*indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);*/

            // Primeiro triângulo
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);

            // Segundo triângulo
            indices.push_back(i1);
            indices.push_back(i3);
            indices.push_back(i2);
        }
    }

    return Mesh(vertices, indices, material);
} 
 
Mesh PrimitiveMesh::Cylinder(unsigned int segments, float height, float radius, bool capped, Material* material) {
    //static std::unordered_map<size_t, Mesh> cached;
    //size_t key = std::hash<std::string>()(std::to_string(segments) + "|" + std::to_string(height) + "|" + std::to_string(radius) + "|" + std::to_string(capped));
    //if (cached.find(key) != cached.end()) return cached[key];

    vector<Vertex> vertices;
    vector<unsigned int> indices;
    float halfHeight = height / 2.0f;

    // Corpo lateral
    for (unsigned int i = 0; i <= segments; ++i) {
        float theta = i * 2.0f * M_PI / segments;
        float x = cos(theta);
        float z = sin(theta);

        glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
        glm::vec3 posTop = glm::vec3(x * radius, halfHeight, z * radius);
        glm::vec3 posBottom = glm::vec3(x * radius, -halfHeight, z * radius);

        float u = float(i) / segments;

        vertices.push_back(Vertex{ posTop, normal, glm::vec2(u, 1.0f) });
        vertices.push_back(Vertex{ posBottom, normal, glm::vec2(u, 0.0f) });

        if (i < segments) {
            unsigned int i0 = i * 2;
            unsigned int i1 = i0 + 1;
            unsigned int i2 = i0 + 2;
            unsigned int i3 = i0 + 3;
            indices.insert(indices.end(), { i0, i1, i2, i2, i1, i3 });
        }
    }

    // Tampas superior/inferior
    if (capped) {
        unsigned int baseIndex = vertices.size();

        // Topo
        vertices.push_back(Vertex{ glm::vec3(0, halfHeight, 0), glm::vec3(0, 1, 0), glm::vec2(0.5f, 0.5f) });
        for (unsigned int i = 0; i <= segments; ++i) {
            float theta = i * 2.0f * M_PI / segments;
            float x = cos(theta), z = sin(theta);
            vertices.push_back(Vertex{
                glm::vec3(x * radius, halfHeight, z * radius),
                glm::vec3(0, 1, 0),
                glm::vec2(0.5f + x * 0.5f, 0.5f + z * 0.5f)
                });
        }
        for (unsigned int i = 1; i <= segments; ++i) {
            indices.insert(indices.end(), { baseIndex, baseIndex + i, baseIndex + i + 1 });
        }

        // Base
        baseIndex = vertices.size();
        vertices.push_back(Vertex{ glm::vec3(0, -halfHeight, 0), glm::vec3(0, -1, 0), glm::vec2(0.5f, 0.5f) });
        for (unsigned int i = 0; i <= segments; ++i) {
            float theta = i * 2.0f * M_PI / segments;
            float x = cos(theta), z = sin(theta);
            vertices.push_back(Vertex{
                glm::vec3(x * radius, -halfHeight, z * radius),
                glm::vec3(0, -1, 0),
                glm::vec2(0.5f + x * 0.5f, 0.5f + z * 0.5f)
                });
        }
        for (unsigned int i = 1; i <= segments; ++i) {
            indices.insert(indices.end(), { baseIndex, baseIndex + i + 1, baseIndex + i });
        }
    }

    return Mesh(vertices, indices, material);
}


//std::unique_ptr<Mesh> PrimitiveMesh::cylinderMesh = nullptr;
//Mesh* PrimitiveMesh::CreateCylinder(unsigned int segments, float height, float radius, bool capped, Material* material) {
//    if (!cylinderMesh) {
//        cylinderMesh = std::make_unique<Mesh>();
//        Mesh mesh = Cylinder(segments, height, radius, material);
//        cylinderMesh->AddMesh(mesh);
//    }
//    return cylinderMesh.get();
//}
const Mesh& PrimitiveMesh::CreateTorus(unsigned int segments, unsigned int ringSegments, float majorRadius, float minorRadius) {
    //static std::unordered_map<size_t, Mesh> cached;
    //size_t key = std::hash<std::string>()(std::to_string(segments) + "|" + std::to_string(ringSegments) + "|" + std::to_string(majorRadius) + "|" + std::to_string(minorRadius));
    //if (cached.find(key) != cached.end()) return cached[key];

    vector<Vertex> vertices;
    vector<unsigned int> indices;

    for (unsigned int i = 0; i <= segments; ++i) {
        float u = float(i) / segments;
        float theta = u * 2.0f * M_PI;
        glm::vec3 circleCenter = glm::vec3(cos(theta), 0, sin(theta)) * majorRadius;

        for (unsigned int j = 0; j <= ringSegments; ++j) {
            float v = float(j) / ringSegments;
            float phi = v * 2.0f * M_PI;
            float x = cos(phi) * minorRadius;
            float y = sin(phi) * minorRadius;

            glm::vec3 normal = glm::vec3(cos(theta) * cos(phi), sin(phi), sin(theta) * cos(phi));
            glm::vec3 pos = circleCenter + normal * minorRadius;
            glm::vec2 tex = glm::vec2(u, v);

            vertices.push_back(Vertex{ pos, glm::normalize(normal), tex });
        }
    }

    for (unsigned int i = 0; i < segments; ++i) {
        for (unsigned int j = 0; j < ringSegments; ++j) {
            unsigned int a = i * (ringSegments + 1) + j;
            unsigned int b = a + ringSegments + 1;

            indices.insert(indices.end(), {
                a, b, a + 1,
                a + 1, b, b + 1
                });
        }
    }

    return Mesh(vertices, indices, nullptr);
}






std::unique_ptr<Mesh> PrimitiveMesh::sphereMeshLight = nullptr;
Mesh* PrimitiveMesh::CreateSphere2RenderingLight(unsigned int xSegments, unsigned int ySegments) {
    if (sphereMeshLight) return sphereMeshLight.get();

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int y = 0; y <= ySegments; ++y) {
        for (unsigned int x = 0; x <= xSegments; ++x) {
            float xSegment = (float)x / (float)xSegments;
            float ySegment = (float)y / (float)ySegments;
            float xPos = std::cos(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());
            float yPos = std::cos(ySegment * glm::pi<float>());
            float zPos = std::sin(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());

            Vertex vertex;
            vertex.Position = glm::vec3(xPos, yPos, zPos);
            vertex.Normal = vertex.Position;
            vertex.TexCoords = glm::vec2(xSegment, ySegment);
            vertices.push_back(vertex);
        }
    }

    for (unsigned int y = 0; y < ySegments; ++y) {
        for (unsigned int x = 0; x < xSegments; ++x) {
            indices.push_back(y * (xSegments + 1) + x);
            indices.push_back((y + 1) * (xSegments + 1) + x);
            indices.push_back((y + 1) * (xSegments + 1) + x + 1);

            indices.push_back(y * (xSegments + 1) + x);
            indices.push_back((y + 1) * (xSegments + 1) + x + 1);
            indices.push_back(y * (xSegments + 1) + x + 1);
        }
    }

    //sphereMeshLight = std::make_unique<Mesh>(true, vertices, indices, nullptr);
    sphereMeshLight = std::make_unique<Mesh>(vertices, indices, nullptr);
    //Mesh* sphere = new Mesh(vertices, indices, nullptr);
    return sphereMeshLight.get();
}
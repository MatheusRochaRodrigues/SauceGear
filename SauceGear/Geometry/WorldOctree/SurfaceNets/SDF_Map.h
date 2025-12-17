#pragma once 
#include "../SDF/SignedDistanceField.h"
#include "../SDF/Terrain.h"
#include "../SDF/Planet.h"
#include "OctreeNode.h"

class SDF_Map {
public:
	SignedDistanceField* sdf;

    SDF_Map() { sdf = new Terrain(); };  //Planet
	~SDF_Map() = default;

    bool has_surface(const OctreeNode& n, float value, float octreeScale)
    {
        // Para decidir se a superfície (SDF ~ 0) cruza este nó da octree,
        // calculamos uma "distância limite" em relação ao centro do nó.

        // 1. Comprimento do lado do nó
        float edgeLength = static_cast<float> (1 << n.depthLOD) * octreeScale;

        // Fórmula da Diagonal do Cubo - d = a*sqrt(3)
        // - d é a medida da diagonal do cubo.
        // - a é a medida da aresta (lado) do cubo. 
        // E para saber a distancia máxima de um vértice diagonal ate o centro do cubo e capturar a superfície é:
        // d_max = (3*(0.5)^3)^(1/3) ≈ 1.44225
        // Multiplicamos por 1.75 como fator de segurança / fudge factor
        // para garantir que nós próximos à superfície sejam subdivididos corretamente.
        // 
        const float surfaceNetThreshold = edgeLength * 1.44224957f * 1.75f;

        //const float surfaceNetThreshold = edgeLength * 0.8660254037 * 1.75f;


        std::cout << "depthLOD " << n.depthLOD << std::endl;
        std::cout << "edgeLength " << (1 << n.depthLOD) << std::endl;
        std::cout << "surfaceNetThreshold " << surfaceNetThreshold << std::endl;
        std::cout << "value " << value << std::endl;
        std::cout << "hs surf " << (std::abs(value) < surfaceNetThreshold) << std::endl;

        // 3. Se o valor absoluto do SDF do centro do nó é menor que o limite,
        // assumimos que a superfície cruza este nó.
        return std::abs(value) < surfaceNetThreshold;
    }

    bool has_surface(float s[8]) {
        bool pos = false, neg = false;
        for (int i = 0; i < 8; i++) {
            pos |= s[i] > 0;
            neg |= s[i] < 0;
        }
        return pos && neg; // tem SDF cruzando
    }

};

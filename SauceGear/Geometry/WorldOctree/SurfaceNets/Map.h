#pragma once 
#include "../SDF/SignedDistanceField.h"
#include "../SDF/Terrain.h"

class Map {
public:
	SignedDistanceField* sdf;

	Map() { sdf = new Terrain(); };
	~Map () = default; 

    bool has_surface(const OctreeNode& n, const float value, const float octreeScale)
    {
        // Para decidir se a superfície (SDF ~ 0) cruza este nó da octree,
        // calculamos uma "distância limite" em relação ao centro do nó.

        // 1. Comprimento do lado do nó
        const float edgeLength = static_cast<float> (1 << n.depthLOD) * octreeScale;

        // Fórmula da Diagonal do Cubo - d = a*sqrt(3)
        // - d é a medida da diagonal do cubo.
        // - a é a medida da aresta (lado) do cubo. 
        // E para saber a distancia máxima de um vértice diagonal ate o centro do cubo e capturar a superfície é:
        // d_max = (3*(0.5)^3)^(1/3) ≈ 1.44225
        // Multiplicamos por 1.75 como fator de segurança / fudge factor
        // para garantir que nós próximos à superfície sejam subdivididos corretamente.
        const float surfaceNetThreshold = edgeLength * 1.44224957f * 1.75f;

        // 3. Se o valor absoluto do SDF do centro do nó é menor que o limite,
        // assumimos que a superfície cruza este nó.
        return std::abs(value) < surfaceNetThreshold;
    }
};

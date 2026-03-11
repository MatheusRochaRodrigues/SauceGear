#pragma once
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <limits>
#include <iostream>

namespace GeneratorMap {
    // Indexação 3D → 1D (linear) // helper linearize function - assumes row-major x + y*nx + z*nx*ny
    inline size_t linearize3(size_t dim, uint32_t x, uint32_t y, uint32_t z) {
        return x + y * dim + z * dim * dim;
    }
    inline size_t linearize3(size_t cellDimension, glm::vec3 d) {
        return size_t(d.x) + size_t(d.y) * cellDimension + size_t(d.z) * cellDimension * cellDimension;    //x + y * sizeX + z * sizeX * sizeY;
    }

    inline std::vector<float> GenerateSphereSDF(int dim, float radius) {
        std::vector<float> sdf(dim * dim * dim);
        glm::vec3 center = glm::vec3(dim / 2.0f); // centro da esfera

        for (int z = 0; z < dim; ++z) {
            for (int y = 0; y < dim; ++y) {
                for (int x = 0; x < dim; ++x) {
                    glm::vec3 p(x, y, z);
                    float dist = glm::length(p - center) - radius;
                    sdf[linearize3(dim, x, y, z)] = dist;
                }
            }
        }

        return sdf;
    }

    inline void DebugPrintSDF(const std::vector<float>& sdf, int dim) {
        int z = dim / 2; // fatia central
        std::cout << "SDF slice at z=" << z << ":\n";

        for (int y = 0; y < dim; ++y) {
            for (int x = 0; x < dim; ++x) {
                float v = sdf[GeneratorMap::linearize3(dim, x, y, z)];
                if (v < 0) std::cout << "##";  // dentro da esfera
                else if (v < 1.0f) std::cout << ".."; // superfície
                else std::cout << "  "; // fora
            }
            std::cout << "\n";
        }
    }

}
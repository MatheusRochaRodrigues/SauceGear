#pragma once
#include "SurfaceNets.h"
#include "../Graphics/Mesh.h"
#include "SurfaceNetsGPUBuffer.h"
#include <glad/glad.h>
#include <vector>
#include <iostream>
#include <cassert>

//#define sysv SysVoxel::getInstance()

struct DebugVoxels {
    static void drawVoxelGrid(glm::vec3 uOffset)
    {
        const int DimVoxel = sysv.get_voxelGrid();
        const float voxelSize = sysv.get_voxelSize();

        // Cor padrão da grade
        glm::vec3 color(0.3f, 0.25f, 0.25f); // cinza escuro
        //float thickness = 1.0f;
        bool thickness = true;

        // desenha linhas do cubo de cada célula
        for (int z = 0; z < DimVoxel - 1; ++z)
            for (int y = 0; y < DimVoxel - 1; ++y)
                for (int x = 0; x < DimVoxel - 1; ++x)
                {
                    glm::vec3 base = uOffset + glm::vec3(x, y, z) * voxelSize;

                    glm::vec3 p000 = base;
                    glm::vec3 p100 = base + glm::vec3(voxelSize, 0, 0);
                    glm::vec3 p010 = base + glm::vec3(0, voxelSize, 0);
                    glm::vec3 p110 = base + glm::vec3(voxelSize, voxelSize, 0);

                    glm::vec3 p001 = base + glm::vec3(0, 0, voxelSize);
                    glm::vec3 p101 = base + glm::vec3(voxelSize, 0, voxelSize);
                    glm::vec3 p011 = base + glm::vec3(0, voxelSize, voxelSize);
                    glm::vec3 p111 = base + glm::vec3(voxelSize, voxelSize, voxelSize);

                    // Linhas do cubo
                    DebugRenderer::AddLine(p000, p100, color, thickness);
                    DebugRenderer::AddLine(p000, p010, color, thickness);
                    DebugRenderer::AddLine(p000, p001, color, thickness);

                    DebugRenderer::AddLine(p100, p110, color, thickness);
                    DebugRenderer::AddLine(p100, p101, color, thickness);

                    DebugRenderer::AddLine(p010, p110, color, thickness);
                    DebugRenderer::AddLine(p010, p011, color, thickness);

                    DebugRenderer::AddLine(p001, p101, color, thickness);
                    DebugRenderer::AddLine(p001, p011, color, thickness);

                    DebugRenderer::AddLine(p110, p111, color, thickness);
                    DebugRenderer::AddLine(p101, p111, color, thickness);
                    DebugRenderer::AddLine(p011, p111, color, thickness);
                }
    }


    static void getDebug3(size_t voxelCount, GLuint computeProgram,
        GLuint gx, GLuint gy, GLuint gz,
        const SurfaceNetsGPUBuffer& gpuBuff,
        glm::vec3 uOffset)
    {
        const int DimVoxel = sysv.get_voxelGrid();
        const float voxelSize = sysv.get_voxelSize();

        // 1️⃣ Ler SDF
        std::vector<float> sdf(voxelCount);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboSDF);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(float), sdf.data());

        // 2️⃣ Ler tabela stride_to_index
        std::vector<uint32_t> strideToIndex(voxelCount);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboStrideToIndex);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(uint32_t), strideToIndex.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // 3️⃣ Desenhar voxels
        for (int z = 0; z < DimVoxel; ++z)
            for (int y = 0; y < DimVoxel; ++y)
                for (int x = 0; x < DimVoxel; ++x)
                {
                    size_t idx = (z * DimVoxel * DimVoxel) + (y * DimVoxel) + x;
                    float val = sdf[idx];

                    glm::vec3 base = uOffset + glm::vec3(x, y, z) * voxelSize;

                    // --- Cor do ponto ---
                    glm::vec3 color;
                    if (strideToIndex[idx] != 0xFFFFFFFFu)
                        color = glm::vec3(0, 1, 0); // verde: cruzamento (tem vértice)
                    else if (val > 0)
                        color = glm::vec3(0.0f, 0.0f, 0.4f); // azul escuro = fora
                    else
                        color = glm::vec3(0.4f, 0.0f, 0.0f); // vermelho escuro = dentro

                    DebugRenderer::AddPoint(base, color, 10.5f, DebugPointType::Square, true);

                    // --- Desenhar grade do voxel (arestas) ---
                    glm::vec3 c000 = base;
                    glm::vec3 c100 = base + glm::vec3(voxelSize, 0, 0);
                    glm::vec3 c010 = base + glm::vec3(0, voxelSize, 0);
                    glm::vec3 c110 = base + glm::vec3(voxelSize, voxelSize, 0);
                    glm::vec3 c001 = base + glm::vec3(0, 0, voxelSize);
                    glm::vec3 c101 = base + glm::vec3(voxelSize, 0, voxelSize);
                    glm::vec3 c011 = base + glm::vec3(0, voxelSize, voxelSize);
                    glm::vec3 c111 = base + glm::vec3(voxelSize, voxelSize, voxelSize);

                    continue;

                    // --- Desenhar só a grade externa (para performance) ---
                    bool isBorder = (x == 0 || y == 0 || z == 0 ||
                        x == DimVoxel - 1 || y == DimVoxel - 1 || z == DimVoxel - 1);

                    if (isBorder)   //isBorder
                    {
                        glm::vec3 gridColor = glm::vec3(0.35f, 0.35f, 0.35f);

                        DebugRenderer::AddLine(c000, c100, gridColor);
                        DebugRenderer::AddLine(c000, c010, gridColor);
                        DebugRenderer::AddLine(c000, c001, gridColor);
                        DebugRenderer::AddLine(c111, c011, gridColor);
                        DebugRenderer::AddLine(c111, c101, gridColor);
                        DebugRenderer::AddLine(c111, c110, gridColor);
                        DebugRenderer::AddLine(c100, c110, gridColor);
                        DebugRenderer::AddLine(c100, c101, gridColor);
                        DebugRenderer::AddLine(c010, c110, gridColor);
                        DebugRenderer::AddLine(c010, c011, gridColor);
                        DebugRenderer::AddLine(c001, c011, gridColor);
                        DebugRenderer::AddLine(c001, c101, gridColor);
                    }
                }
    }



    static void getDebug2(size_t voxelCount, GLuint computeProgram,
        GLuint gx, GLuint gy, GLuint gz,
        const SurfaceNetsGPUBuffer& gpuBuff,
        glm::vec3 uOffset)
    {
        const int DimVoxel = sysv.get_voxelGrid();
        const float voxelSize = sysv.get_voxelSize();

        // 1️⃣ Ler SDF
        std::vector<float> sdf(voxelCount);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboSDF);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(float), sdf.data());

        // 2️⃣ Ler tabela stride_to_index (cada voxel => índice do vértice se existir)
        std::vector<uint32_t> strideToIndex(voxelCount);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuff.ssboStrideToIndex);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, voxelCount * sizeof(uint32_t), strideToIndex.data());

        // 3️⃣ Plotar pontos debug
        for (int z = 0; z < DimVoxel; ++z)
            for (int y = 0; y < DimVoxel; ++y)
                for (int x = 0; x < DimVoxel; ++x)
                {
                    size_t idx = (z * DimVoxel * DimVoxel) + (y * DimVoxel) + x;
                    float val = sdf[idx];

                    glm::vec3 worldPos = uOffset + glm::vec3(x, y, z) * voxelSize;

                    glm::vec3 color;
                    if (strideToIndex[idx] != 0xFFFFFFFFu)
                        color = glm::vec3(0, 1, 0); // verde = há vértice
                    else if (val > 0)
                        color = glm::vec3(0.0f, 0.0f, 0.4f); // azul = fora
                    else
                        color = glm::vec3(0.4f, 0.0f, 0.0f); // vermelho = dentro

                    DebugRenderer::AddPoint(worldPos, color, 6.0f, DebugPointType::Square, true);
                }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }



    static void getDebugCentroid( std::vector<glm::vec4> positions ) {  
        for (auto pos : positions) DebugRenderer::AddPoint(glm::vec3(pos.x, pos.y, pos.z), glm::vec3(0.6f, 0.6f, 0.2f), 12.0f, DebugPointType::Circle, true); 
    }



    // Functor de hash para tuple<int,int,int>
    struct TupleHash {
        size_t operator()(const std::tuple<int, int, int>& t) const noexcept {
            auto [x, y, z] = t;
            // Combina os 3 inteiros em um hash único
            size_t h1 = std::hash<int>()(x);
            size_t h2 = std::hash<int>()(y);
            size_t h3 = std::hash<int>()(z);
            return h1 ^ (h2 << 1) ^ (h3 << 2); // combinação simples e eficiente
        }
    };


    static void getDebug(uint32_t voxelCount, GLuint computeProgram, GLuint gx, GLuint gy, GLuint gz) {
        glUniform1i(glGetUniformLocation(computeProgram, "uPass"), 3);
        // === Buffers de debug ===
        const size_t maxDebugPoints = voxelCount * 64; // cantos + edges
        GLuint ssboDebugPos = SurfaceNetsGPUBuffer::CreateSSBO(sizeof(glm::vec4) * maxDebugPoints);
        GLuint ssboDebugCol = SurfaceNetsGPUBuffer::CreateSSBO(sizeof(glm::vec4) * maxDebugPoints);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssboDebugPos);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssboDebugCol);

        GLuint ssboDebugCounter = SurfaceNetsGPUBuffer::CreateSSBO(sizeof(GLuint));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, ssboDebugCounter);
        GLuint zero = 0; glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &zero);

        glDispatchCompute(gx, gy, gz);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        // === Ler debug data ===
        GLuint debugCount = 0;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDebugCounter);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &debugCount);

        if (debugCount > maxDebugPoints) debugCount = maxDebugPoints;   //n entendi

        std::vector<glm::vec4> dbgPositions(debugCount);
        std::vector<glm::vec4> dbgColors(debugCount);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDebugPos);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * debugCount, dbgPositions.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDebugCol);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * debugCount, dbgColors.data());

        for (size_t i = 0; i < dbgPositions.size(); ++i) {
            glm::vec3 pos = glm::vec3(dbgPositions[i]);
            glm::vec3 col = glm::vec3(dbgColors[i]);
            DebugRenderer::AddPoint(pos, col, 11.0f, DebugPointType::Square, true);
        }

        /*
        for (size_t i = 0; i + 1 < dbgPositions.size(); i += 2) {
            glm::vec3 a =   glm::vec3(dbgPositions[i]);
            glm::vec3 b =   glm::vec3(dbgPositions[i + 1]);
            glm::vec3 col = glm::vec3(dbgColors[i]);
            DebugRenderer::AddLine(a, b, col, true);
        }
        */



        //All samples 
        /*
        // função para transformar glm::vec3 em chave discreta
        auto posToKey = [](const glm::vec3& p) {
            return std::make_tuple(int(std::round(p.x)), int(std::round(p.y)), int(std::round(p.z)));
        };


        // criar set de pontos ocupados
        std::unordered_set<std::tuple<int, int, int>, TupleHash> occupied;

        // preencher com pontos do shader
        for (auto& p : dbgPositions) {
            occupied.insert(posToKey(glm::vec3(p)));
        }

        glm::vec3 numChunks = glm::vec3(sysv.get_voxelGrid());
        for (int cz = 0; cz < numChunks.z; ++cz) for (int cy = 0; cy < numChunks.y; ++cy) for (int cx = 0; cx < numChunks.x; ++cx)
        {
            glm::vec3 offset = glm::vec3(cx, cy, cz) * (float)sysv.get_voxelSize();
            if (occupied.find(posToKey(offset)) == occupied.end()) //not exists
            {
                // ponto não existe, adicionar como preto
                DebugRenderer::AddPoint(offset, glm::vec3(0.0f), 11.0f, DebugPointType::Square, true);
            }
        }
        */

        // === Opcional: deletar SSBOs depois se não for reaproveitar ===
        glDeleteBuffers(1, &ssboDebugPos);
        glDeleteBuffers(1, &ssboDebugCol);
        glDeleteBuffers(1, &ssboDebugCounter);
    }


};
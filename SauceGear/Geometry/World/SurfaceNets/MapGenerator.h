#pragma once 
#include "../Graphics/ComputeShader.h" 
#include "../Geometry/World/SurfaceNets/SurfaceNets.h"   

class GPUMapGenerator {
public:
    ComputeShader compute; 
    GPUMapGenerator(const char* shaderFile) : compute(shaderFile) {   } 

    void Generate(glm::vec3 worldOffset, ChunkBuffer& buff, SurfaceNetsGPUBuffer& destGpuBuf, int dim, float vSize) {
        // LOD-aware 
        size_t dim3 = size_t(dim) * dim * dim;

        // redimensiona CPU buffer
        if (buff.densityMap.size() != dim3)
            buff.densityMap.resize(dim3, 1.0f);

        // assegura capacidade do GPU buffer
        destGpuBuf.ensureCapacity();

        // set uniforms
        glUseProgram(compute.ID);
        glUniform1i(glGetUniformLocation(compute.ID, "uDim"), dim);
        glUniform1f(glGetUniformLocation(compute.ID, "uVoxelSize"), vSize);
        glUniform1f(glGetUniformLocation(compute.ID, "uIsoLevel"), 0);  //sysv.isoLevel
        glUniform3f(glGetUniformLocation(compute.ID, "uOffset"), worldOffset.x, worldOffset.y, worldOffset.z);

        // bind SSBO
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, destGpuBuf.ssboSDF);

        // despacha compute shader para gerar SDF
        GLuint gx = (dim + 7) / 8;
        GLuint gy = gx;
        GLuint gz = gx;

        compute.Dispatch(gx, gy, gz, true); // true = espera a conclusão

        // lê SDF de volta para CPU
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, destGpuBuf.ssboSDF);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, dim3 * sizeof(float), buff.densityMap.data());
        //glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        std::cout << "SDF gerado no GPUBuffer " << destGpuBuf.ssboSDF
            << " para offset (" << worldOffset.x << ", " << worldOffset.y << ", " << worldOffset.z << ") "
            << "dim=" << dim << " voxelSize=" << vSize << std::endl;
    }


};





//GLuint ssbo = 0; size_t size_ssbo = 0;
 
/*      Init(sysv.get_voxelGrid());
// inicializa o SSBO
void Init(int dim) {
    // cria SSBO para SDF
    if (ssbo == 0) glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, dim * dim * dim * sizeof(float), nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    size_ssbo = size_t(dim) * dim * dim;
}
*/

//if (dim3 != size_ssbo) Init(dim);

/*
~GPUMapGenerator() {
    if (ssbo) glDeleteBuffers(1, &ssbo);
}
*/




/*
// do not read back to CPU; if destGpuBuf provided and has SSBO, we can copy GPU->GPU:
if (destGpuBuf) {
    // Ensure dest capacity and then copy buffer (glCopyBufferSubData)
    destGpuBuf->ensureCapacity(dim3); // must be GL thread
    glBindBuffer(GL_COPY_READ_BUFFER, ssbo);
    glBindBuffer(GL_COPY_WRITE_BUFFER, destGpuBuf->ssboSDF);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, dim3 * sizeof(float));
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    // Note: now destGpuBuf has SDF; caller can pass destGpuBuf->ssboSDF to SurfaceNetsGPU::Generate with sdfAlreadyOnGPU=true
    return;
} else {
    // leave SDF in this->ssbo; caller may pass this->ssbo upward
    return;
}
*/


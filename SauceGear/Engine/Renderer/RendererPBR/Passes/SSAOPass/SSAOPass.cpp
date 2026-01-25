#include "SSAOPass.h"
#include "../../../../Graphics/Texture.h"
#include <random>

static float ourLerp(float a, float b, float f)     ////ourLerp
{ 
    return a + f * (b - a); 
}     

void SSAOPass::GenerateKernel() {
    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    std::default_random_engine gen;

    kernel.clear();
    for (int i = 0; i < iSamples; ++i) {  //64
        glm::vec3 sample(
            rnd(gen) * 2.0f - 1.0f,
            rnd(gen) * 2.0f - 1.0f,
            rnd(gen)
        );
        sample = glm::normalize(sample);
        sample *= rnd(gen);

        // Otimizar a distribuição do kernel de amostragem: alterar de distribuição aleatória uniforme para uma concentração maior de amostras 
        // próximas à origem (perto do fragmento), utilizando interpolação acelerada.
        //
        // scale samples s.t. they're more aligned to center of kernel
        float scale = float(i) / (float)iSamples;               // 64.0f
        scale = ourLerp(0.1f, 1.0f, scale * scale);    //equivalente a  ->  glm::mix(0.1f, 1.0f, scale * scale);
        kernel.push_back(sample * scale);
    }
}

void SSAOPass::GenerateNoise() {
    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    std::default_random_engine gen;

    std::vector<glm::vec3> noise;
    for (int i = 0; i < 16; i++) {
        // rotate around z-axis (in tangent space)
        noise.emplace_back(
            rnd(gen) * 2.0f - 1.0f,
            rnd(gen) * 2.0f - 1.0f,
            0.0f
        );
    }

    noiseTexture = new Texture(
        4, 4,
        GL_RGBA32F, GL_RGB, GL_FLOAT,       //GL_RGBA16F
        GL_REPEAT, GL_NEAREST,
        noise.data()   // ou &ssaoNoise[0]
    );      
}

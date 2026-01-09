#include "CascadeSun.h"
#include "../../../Graphics/Renderer.h"

static std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    glm::mat4 inv = glm::inverse(proj * view);
    std::vector<glm::vec4> corners;
    corners.reserve(8);
    //corners cube - centrados em 0, logo um canto fica em -1 e outro em 1
    for (int x = 0; x < 2; x++)
        for (int y = 0; y < 2; y++)
            for (int z = 0; z < 2; z++) {
                glm::vec4 pt = inv * glm::vec4(
                    2.0f * x - 1.0f,
                    2.0f * y - 1.0f,
                    2.0f * z - 1.0f,
                    1.0f);
                corners.push_back(pt / pt.w);
            }
    return corners;
}

void CascadeSun::GetLightSpaceMatrices(TransformComponent& transform)
{
    auto camera = GEngine->mainCamera;
    if (!camera) return;

    float nearClip = camera->nearClip;
    float farClip = camera->farClip;
    glm::mat4 view = camera->GetViewMatrix();

    size_t splits = cascadePlaneDistances.size();
    if (lightSpaceMatrices.size() < splits) lightSpaceMatrices.resize(splits);

    // garante que world matrix e rotation estejam corretas 
    transform.UpdateWorldAsRoot();  //transform.UpdateLocalMatrixIfNeeded();
    glm::vec3 lightDir = glm::normalize(transform.GetForwardDirection());      //-transform.GetForwardDirection()

    for (size_t i = 0; i < splits; ++i)
    {
        float nearPlane = (i == 0) ? nearClip : cascadePlaneDistances[i - 1];
        float farPlane = cascadePlaneDistances[i];

        glm::mat4 proj = glm::perspective(glm::radians(camera->Zoom), camera->aspectRatio, nearPlane, farPlane); 
        auto corners = GetFrustumCornersWorldSpace(proj, view);

        // centro do frustum
        glm::vec3 center(0.0f);
        for (auto& v : corners) center += glm::vec3(v);
        center /= corners.size();

        //glm::mat4 lightView = glm::lookAt(center - lightDir * 50.0f, center, glm::vec3(0, 1, 0));
        //glm::mat4 lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));
        //glm::mat4 lightView = glm::lookAt(center + lightDir * 40.0f, center, glm::vec3(0.0f, 1.0f, 0.0f));
        //glm::mat4 lightView = glm::lookAt(center - lightDir * 50.0f, center, glm::vec3(0.0f, 1.0f, 0.0f));
        //glm::mat4 lightView = glm::lookAt(center + lightDir * 10.0f, center, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightView = glm::lookAt(center + lightDir * 10.0f, center, glm::vec3(0.0f, 1.0f, 0.0f));

        //----------- Teste
        //glm::vec3 objectCenter = glm::vec3(0.0f, 0.0f, 0.0f); // centro do objeto
        //glm::vec3 lightPos = objectCenter + glm::normalize(glm::vec3(20.0f, 50, 20.0f)); // posição da luz fixa
        //glm::mat4 lightView = glm::lookAt(lightPos, center, glm::vec3(0, 1, 0));
        //-----------

        float minX = FLT_MAX, maxX = -FLT_MAX;
        float minY = FLT_MAX, maxY = -FLT_MAX;
        float minZ = FLT_MAX, maxZ = -FLT_MAX;

        for (auto& v : corners)
        {
            glm::vec4 trf = lightView * v;
            minX = std::min(minX, trf.x);
            maxX = std::max(maxX, trf.x);
            minY = std::min(minY, trf.y);
            maxY = std::max(maxY, trf.y);
            minZ = std::min(minZ, trf.z);
            maxZ = std::max(maxZ, trf.z);
        }

        // aplica margem para evitar cortes
        const float zMult = 10.0f;
        if (minZ < 0) minZ *= zMult; else minZ /= zMult;
        if (maxZ < 0) maxZ /= zMult; else maxZ *= zMult;

        glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
        lightSpaceMatrices[i] = lightProjection * lightView;
    }
}

void CascadeSun::Init() {
    if (initialized) return;
    initialized = true;

    glGenFramebuffers(1, &cascadeFBO);
    glGenTextures(1, &cascadeDepthMapArray);

    int cascadeResolution = 2048;
    const int layers = CASCADE_COUNT; // N cascades -> layers/matrices

    glBindTexture(GL_TEXTURE_2D_ARRAY, cascadeDepthMapArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
        cascadeResolution, cascadeResolution, layers,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0,1.0,1.0,1.0 };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Bind temporariamente (não anexamos layer aqui — usaremos glFramebufferTextureLayer ao renderizar)
    glBindFramebuffer(GL_FRAMEBUFFER, cascadeFBO);

    // attach something minimal so framebuffer is valid (we will attach each layer per-pass)
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D_ARRAY, cascadeDepthMapArray, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cascadeDepthMapArray, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "[CascadeSun] framebuffer incomplete: " << status << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // UBO agora com layers matrizes
    glGenBuffers(1, &cascadeMatricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, cascadeMatricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * layers, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, cascadeMatricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    lightSpaceMatrices.resize(layers);            
    cascadePlaneDistances.resize(layers);          
}



void CascadeSun::UpdateSunShadow(LightComponent& sun, TransformComponent& transform) {
    if (!initialized) Init();

    auto camera = GEngine->mainCamera;
    if (!camera) return;

    float farClip = camera->farClip;
    // Define splits
    cascadePlaneDistances[0] = farClip / 50.0f;
    cascadePlaneDistances[1] = farClip / 25.0f;
    cascadePlaneDistances[2] = farClip / 10.0f;
    cascadePlaneDistances[3] = farClip / 2.0f;
    cascadePlaneDistances[4] = farClip; //ate o fim da camera
     
    GetLightSpaceMatrices(transform); 

    // envia UBO
    glBindBuffer(GL_UNIFORM_BUFFER, cascadeMatricesUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * lightSpaceMatrices.size(), lightSpaceMatrices.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Renderiza cascades
    glViewport(0, 0, 2048, 2048);
    glBindFramebuffer(GL_FRAMEBUFFER, cascadeFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);  // peter panning
     
    Shader* shadowShader = GEngine->renderer->GetShadowShader_Sun;
    shadowShader->use(); 
    GEngine->renderer->RenderSceneWithShader(shadowShader);  

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GEngine->window->SetWindowViewport0();
}

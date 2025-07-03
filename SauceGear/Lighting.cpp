#include "Lighting.h"

Lighting::Lighting(
    //Resolution
    Shader& Directional, Shader& Point,
    const unsigned int SHADOW_WIDTH, const unsigned int SHADOW_HEIGHT
    ) 
    : SHADOW_WIDTH(SHADOW_WIDTH), SHADOW_HEIGHT(SHADOW_HEIGHT)  {

    glGenFramebuffers(1, &depthMapFBO);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); 
    // are clearly defining here that we do not want to draw any color information
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    // returning to the default frame buffer state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // build and compile shaders 
    DirectionalShader = &Directional;
    PointShader = &Point;
    //sPointMap = new Shader(adsPointVs, adsPointFs);
    //Shader debugDepthQuad("3.1.3.debug_quad.vs", "3.1.3.debug_quad_depth.fs");
}

void Lighting::InstanceDirectionalLight(glm::vec3 position, glm::mat4 model, glm::vec3 color) {
    Light l;
    GLuint* depthMap = &l.depthMap;

    // create depth texture 
    glGenTextures(1, depthMap);                //&depthMap
    glBindTexture(GL_TEXTURE_2D, *depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // setup mapping interpolation tex
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    l.position = position;
    l.model = model;
    l.color = color;
    l.type =  Light::TypeLight::Directional;

    allLights.push_back(l);
}

void Lighting::InstancePointLight(glm::vec3 position, glm::mat4 model, glm::vec3 color) {
    Light l;
    GLuint* depthCubemap = &l.depthMap;

    // create depth cubemap texture
    glGenTextures(1, depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)   
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    l.position = position;
    l.model = model;
    l.color = color;
    l.type = Light::TypeLight::Point;

    allLights.push_back(l);
}
 

//auto nearestLights = getLightsNearest(positionPlayer); // pega as 8 mais próximas
void Lighting::UpdateLights(glm::vec3 positionPlayer) {
    // usa "elemento" diretamente, que é uma referência para cada item
    for (auto& light : allLights) {
        if (light.depthMap == 0) {
            std::cerr << "Erro: light.depthMap não inicializado!\n";
            continue;
        } 
        if (light.type == Light::TypeLight::Directional)
            updtDirectional(light);
        else
            updtPoint(light); 
    }  
}

void Lighting::updtDirectional(Light& light) {
    glm::vec3 lightPos = light.position;
    Shader* simpleDepthShader = DirectionalShader;

    // 1. render depth of scene to texture (from light's perspective)
    // --------------------------------------------------------------
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    // Projection
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    // View
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    // light Space for render
    lightSpaceMatrix = lightProjection * lightView;

    // render scene from light's point of view
    simpleDepthShader->use();
    simpleDepthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light.depthMap, 0);

    // returning to the default frame buffer state
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    //glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, woodTexture);
    //renderScene(simpleDepthShader);

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //gambiarra
    light.lightSpaceMatrices = lightSpaceMatrix;

}

void Lighting::updtPoint(Light& light) {
    glm::vec3 lightPos = light.position;
    Shader* simpleDepthShader = PointShader;


    // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
    float near_plane = 1.0f;
    float far_plane = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
    
    light.shadowTransforms[0] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(0.0f, -1.0f, 0.0f));
    light.shadowTransforms[1] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    light.shadowTransforms[2] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 0.0f,  1.0f));    
    light.shadowTransforms[3] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    light.shadowTransforms[4] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, -1.0f, 0.0f));
    light.shadowTransforms[5] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

    // 1. render scene to depth cubemap
        // --------------------------------
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    // attach depth texture as FBO's depth buffer 
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light.depthMap, 0);
    glClear(GL_DEPTH_BUFFER_BIT);

    simpleDepthShader->use();
    for (unsigned int i = 0; i < 6; ++i)
        simpleDepthShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", light.shadowTransforms[i]);
    simpleDepthShader->setFloat("far_plane", far_plane);
    simpleDepthShader->setVec3("lightPos", lightPos);


    //renderScene(simpleDepthShader);


    // returning to the default frame buffer state
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
      

    //gambiarra desnecessaria
    //light.lightSpaceMatrices = glm::mat4(1.0f);
    
     
    // Projection
    //note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene;
    //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane);
     
}




std::vector<Light*> Lighting::getLightsNearest(glm::vec3 pos) {
    // vetor temporário com ponteiros pra todas as luzes
    std::vector<Light*> lightsPtrs;
    lightsPtrs.reserve(allLights.size());

    for (auto& light : allLights)
        lightsPtrs.push_back(&light);

    // ordena por distância crescente para pos
    std::sort(lightsPtrs.begin(), lightsPtrs.end(), [&](Light* a, Light* b) {
        float distA = glm::distance(a->position, pos);
        float distB = glm::distance(b->position, pos);
        return distA < distB;
    });

    // retorna as maxLights primeiras (ou menos se não tiver)
    if ((int)lightsPtrs.size() > MAX_LIGHTS)
        lightsPtrs.resize(MAX_LIGHTS);

    return lightsPtrs;
     
}

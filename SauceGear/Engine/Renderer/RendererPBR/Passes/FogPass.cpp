#include "FogPass.h"
#include "../../../Graphics/Framebuffer.h"    
#include "../../../Graphics/Shader.h"    
#include "../../RenderDebugSettings.h"
#include "../../../Graphics/FullscreenQuad.h"

void FogPass::Execute(Framebuffer* target, Framebuffer* gbuffer, glm::vec3 camPos) { 
    auto& fog = GetEngineSettings().renderDebug;          //auto& fog = scene.GetFogSettings(); 

    target->Bind();
    glDisable(GL_DEPTH_TEST);

    shader->use();

    shader->setInt("u_SceneColor", 0);
    shader->setInt("u_Position", 1);        //shader->setInt("u_Depth", 1);

    shader->setVec3("u_CamPos", camPos);

    shader->setFloat("u_FogDensity", fog.FogDensity);
    shader->setFloat("u_FogStart", fog.FogNear);
    shader->setFloat("u_FogEnd", fog.FogFar);
    shader->setVec3("u_FogColorNear", fog.FogColorNear);
    shader->setVec3("u_FogColorFar", fog.FogColorFar);

    //shader->setMat4("u_InvProjection", scene.camera.GetInvProjection());
    //shader->setMat4("u_InvView", scene.camera.GetInvView());

    //target->GetTexture(0)->Bind(0);
    //gbuffer->GetDepthTexture()->Bind(1);

    glActiveTexture(GL_TEXTURE0);  glBindTexture(GL_TEXTURE_2D, target->GetTexture(0));
    glActiveTexture(GL_TEXTURE1);  glBindTexture(GL_TEXTURE_2D, gbuffer->GetTexture(0));      //gbuffer->GetDepthTexture()

    RenderQuad();

    glEnable(GL_DEPTH_TEST);
}

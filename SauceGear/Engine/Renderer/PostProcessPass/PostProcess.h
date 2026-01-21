#pragma once
#include "../../Scene/SceneECS.h"
#include "../../Graphics/Renderer.h"
#include "PostProcessPass.h" 
#include "Pass/Fix_HDR_GAMMA_pp.h"

class PostProcess {
public:
    PostProcess();
    void initialize();  

    void Execute(Scene& scene, Framebuffer& ppFBO) { 
        ppFBO.Bind();

        for (auto& pp : passes) {  
            pp->shader->use();
            pp->Apply();  // chama a lógica individual de cada efeito    
              
            // define local texture in shader
            pp->shader->setInt("scene", 0);
            glActiveTexture(GL_TEXTURE0);   
            // set texture render scene
            glBindTexture(GL_TEXTURE_2D, GEngine->renderer->GetTextureRendered);

            glDisable(GL_DEPTH_TEST);
            DrawQuad();
            glEnable(GL_DEPTH_TEST);

        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
        //Finish();
    }

    void correct_HDR_GAMA(Framebuffer& ppFBO) {
        static Fix_HDR_GAMMA_pp* correct = nullptr;
        if (correct == nullptr) correct = new Fix_HDR_GAMMA_pp();

        ppFBO.Bind();
        correct->shader->use();   
        correct->shader->setInt("scene", 0); // define local texture in shader
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, GEngine->renderer->GetTextureRendered);    // set texture render scene

        glDisable(GL_DEPTH_TEST);
        DrawQuad();
        glEnable(GL_DEPTH_TEST);  

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
    }

    void Finish(Framebuffer& ppFBO, bool fixToneGamma = true) {
        if(fixToneGamma) correct_HDR_GAMA(ppFBO);

        //===================================== FINALE =============================================================
        //GAME_RUNTIME
        #ifdef EDITOR_BUILD
            //GEngine->renderer->GetTextureRendered = inputTexture;
        #else
            // second pass
            glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glDisable(GL_DEPTH_TEST);
            shaderView->use();
            shaderView->setInt("scene", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, GEngine->renderer->GetTextureRendered);
            DrawQuad();
            glEnable(GL_DEPTH_TEST);
        #endif
    }

    void InitFullscreenQuad(); // cria quad NDC

    void Resize(int width, int height);
    void AddPass(PostProcessPass* pass);
    GLuint GetFinalTexture() const;

private:   
    int width, height;
    std::vector<std::unique_ptr<PostProcessPass>> passes;

    GLuint quadVAO = 0, quadVBO = 0;
    void DrawQuad();
    Shader* shaderView;
};




 

    //void Update(float dt) override {
    //    //if (PostProcessPass::outputFBO == nullptr) initialize();

    //    SceneECS& scene = *GEngine->scene;
    //    GLuint& inputTexture = GEngine->renderer->GetTextureRendered;

    //    for (auto& pp : passes) {
    //        if (pp->outputFBO == nullptr) initialize(*pp);

    //        pp->shader->use();
    //        pp->Apply();  // chama a lógica individual de cada efeito    

    //        // roda o frame buffer e injeta a textura da cena para o post process
    //        glBindFramebuffer(GL_FRAMEBUFFER, pp->outputFBO->GetID());
    //        glClear(GL_COLOR_BUFFER_BIT);
    //        // define local texture in shader
    //        pp->shader->setInt("scene", 0);
    //        glActiveTexture(GL_TEXTURE0);
    //        // set texture render scene
    //        glBindTexture(GL_TEXTURE_2D, inputTexture);
    //        DrawQuad();             //Render();               //FullscreenQuad
    //        //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //        inputTexture = pp->GetOutputTexture();
    //    } 

    //    //debug test Shadow
    //    /*for (auto& [lightEntity, lodMapShadow] : LightSystem::ShadowMaps) {
    //        auto& light = GEngine->scene->GetComponent<LightComponent>(lightEntity);
    //        GEngine->renderer->GetTextureRendered = light.depthMap;
    //    }*/

    //    //GAME_RUNTIME
    //    #ifdef EDITOR_BUILD
    //        GEngine->renderer->GetTextureRendered = inputTexture;
    //    #else
    //        // second pass
    //        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
    //        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    //        glClear(GL_COLOR_BUFFER_BIT);
    //        shaderView->use();
    //        glDisable(GL_DEPTH_TEST);
    //        glBindTexture(GL_TEXTURE_2D, inputTexture);
    //        DrawQuad();
    //        glEnable(GL_DEPTH_TEST);
    //    #endif
    //}

     
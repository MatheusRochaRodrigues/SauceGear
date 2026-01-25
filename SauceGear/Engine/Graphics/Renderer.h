#pragma once
#include "../Scene/SceneECS.h"
#include "../Core/Camera.h"
#include "../Platform/Window.h" 
#include "../Graphics/Framebuffer.h" 
#include "../Graphics/PrimitiveMesh.h"    
#include "../Graphics/Shader.h"    

//backend de renderização.
class Renderer {
public:
    /*enum class RenderPass {
        Forward,
        Shadow,
        DeferredGBuffer,
        Lighting
    };*/
      
    void Init(SceneECS*);

    void Initialize();
    void BeginFrame();
    void Render(Shader shader); 

    void RenderSceneWithShader(Shader* overrideShader); 
    static void RenderSceneWithShader2(Shader* overrideShader);  

private:  
    SceneECS* m_Scene;
    Shader pbrShader;
    Shader baseColorShader; 
       
public:   
    //FrameScreenResult
    GLuint GetTextureRendered;             //GLuint* GetTextureRendered;
    Framebuffer* frameScreen;
     
};








//void ResizeFramebuffer(int width, int height) {
//    framebufferWidth = width;
//    framebufferHeight = height;

//    // Bind framebuffer
//    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

//    // Redefine a textura de cor
//    glBindTexture(GL_TEXTURE_2D, colorTextureID);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTextureID, 0);

//    // Redefine o renderbuffer de profundidade
//    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbufferID);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderbufferID);

//    // Checagem final
//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//        std::cerr << "Framebuffer incompleto após redimensionamento!" << std::endl;

//    // Desvincula
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}
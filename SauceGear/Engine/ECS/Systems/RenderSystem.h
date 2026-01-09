#pragma once 
#include "../../Core/EngineContext.h"
#include "../../Renderer/IRenderPipeline.h" 
#include "../../Renderer/RendererPBR/PBRPipeline.h"
#include "../../Renderer/RendererBlinnPhong/BlinnPhongRenderer.h"

class RenderSystem : public System {
public:   
    RenderSystem() { 
        SetPipeline(std::make_unique<PBRPipeline>());               //SetPipeline(std::make_unique<BlinnPhongPipeline>());
    }

    void Update(float dt) override {  
        try {  
            Render();
        } catch (const std::exception& e) {
            std::cerr << "[EXCEÇÃO - RenderSystem] " << e.what() << "\n";
        } 
        //std::cout << "FPS: " << 1.0f / Time::GetDeltaTime() << std::endl; //std::cout << "FPS: " << 1.0f / dt << std::endl;
    }  

    void Render() {         //const Scene& scene
        if (!pipeline) return;
        pipeline->Render(*GEngine->scene);
    } 

    void SetPipeline(std::unique_ptr<IRenderPipeline> newPipeline) {
        if (pipeline) pipeline->Shutdown();
        pipeline = std::move(newPipeline); 
    }

private:
    std::unique_ptr<IRenderPipeline> pipeline;
};


 
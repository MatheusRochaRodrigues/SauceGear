#pragma once 
#include "../../Core/EngineContext.h"
#include "../../Graphics/Renderer/IRenderPipeline.h" 
#include "../Graphics/Renderer/RenderHelper.h"

class RenderSystem : public System {
public:   
    RenderSystem() { 
        //SetPipeline(std::make_unique<BlinnPhongPipeline>());
        SetPipeline(std::make_unique<PBRPipeline>());
    }

    void Update(float dt) override {
        //RENDER
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
        //pipeline->Init();
    }

private:
    std::unique_ptr<IRenderPipeline> pipeline;
};


 
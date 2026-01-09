#pragma once  
#include "../../../Graphics/Framebuffer.h"   
#include "../../../Graphics/Renderer.h" 

using Scene = SceneECS; 

class GeometryPass {
public:
    GeometryPass(Shader* shader) : shader(shader) {};

    void Execute(Scene& scene, Framebuffer& gbuffer)
    {
        gbuffer.Bind(); 
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     /// glClearColor(0, 0, 0, 1);
         
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        shader->use();

        for (auto e : scene.GetEntitiesWith<MeshRenderer, TransformComponent>()) {
            auto& tr = scene.GetComponent<TransformComponent>(e);
            auto& mr = scene.GetComponent<MeshRenderer>(e);

            shader->setMat4("model", tr.GetMatrix());

            mr.Draw();
        }

        gbuffer.Unbind();
    }

private:
    Shader* shader;
};



/*
auto model = AssetDatabase::Load<ModelAsset>("Models/house.fbx");

auto instance = std::make_shared<ModelInstance>(model);
instance->transform.position = {0,0,0};

Scene::Add(instance);



for (auto& inst : Scene::Models()) {
    DrawNode(inst->asset->root, inst->transform.Matrix(), inst->asset);
}



void DrawNode(const ModelNode& node, mat4 parent, ModelAsset* asset) {
    mat4 world = parent * node.localTransform;

    for (auto idx : node.meshIndices) {
        auto& mesh = asset->meshes[idx];
        for (auto& sm : mesh->submeshes) {
            MaterialBinder::Bind(sm.materialAsset);
            shader->Set("u_Model", world);
            mesh->DrawSubmesh(sm);
        }
    }

    for (auto& c : node.children)
        DrawNode(c, world, asset);
}


*/



/*

void Execute(RenderQueue& rq) {

            for (auto& [_, batch] : rq.GetBatches()) {

                auto& key = batch.key;

                Shader* shader = key.shader;
                shader->Bind();

                MaterialBinder::Bind(
                    *dummyInstance,
                    *key.material,
                    *key.material->base
                );

                MeshInstance* gpu = key.mesh->GetInstance();

                if (batch.transforms.size() == 1) {
                    shader->setMat4("u_Model", batch.transforms[0]);
                    gpu->DrawSubmesh(key.submesh);
                }
                else {
                    gpu->UploadInstanceMatrices(batch.transforms);
                    gpu->DrawSubmeshInstanced(
                        key.submesh,
                        (uint32_t)batch.transforms.size()
                    );
                }
            }
        }

        MaterialInstance* dummyInstance = nullptr;

*/
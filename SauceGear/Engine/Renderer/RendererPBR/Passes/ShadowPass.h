#pragma once
/*

class ShadowPass {
public:
    void Execute(RenderQueue& rq, const glm::mat4& lightVP) {

        shadowShader->Bind();
        shadowShader->setMat4("u_LightVP", lightVP);

        for (auto& [_, batch] : rq.GetBatches()) {

            MeshInstance* gpu = batch.key.mesh->GetInstance();

            gpu->UploadInstanceMatrices(batch.transforms);

            gpu->DrawSubmeshInstanced(
                batch.key.submesh,
                (uint32_t)batch.transforms.size()
            );
        }
    }

    Shader* shadowShader;
};


*/
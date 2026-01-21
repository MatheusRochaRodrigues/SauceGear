#pragma once
#include "../PostProcessPass.h"

class Fix_HDR_GAMMA_pp : public PostProcessPass {
public:
    Fix_HDR_GAMMA_pp() {
        shader = new Shader("PostProcess/post.vs", "PostProcess/Correct_HDR_GAMA.fs");
    };

    void Apply() override {};
};

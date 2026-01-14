#pragma once
#include "../PostProcessPass.h"

class BaW_pp : public PostProcessPass {
public:
    BaW_pp() {
        shader = new Shader("PostProcess/post.vs", "PostProcess/BlackAndWhite.fs");
    }; 

    void Apply() override {} ;
};
 
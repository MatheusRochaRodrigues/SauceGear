#pragma once

class RenderGraph;

class RenderPass {
public:
    virtual void Setup(RenderGraph&) {}
    virtual void Execute() = 0;
    virtual ~RenderPass() = default;
};

#pragma once
#include <vector>
#include <memory>

class RenderPass;

class RenderGraph {
public:
    template<typename T, typename... Args>
    T* AddPass(Args&&... args) {
        auto p = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = p.get();
        passes.push_back(std::move(p));
        return ptr;
    }

    void Execute() {
        for (auto& p : passes)
            p->Execute();
    }

private:
    std::vector<std::unique_ptr<RenderPass>> passes;
};

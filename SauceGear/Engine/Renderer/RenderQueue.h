#pragma once
#include "RenderItem.h"
#include "../Graphics/Shader.h"

class RenderQueue {
public:
    void Clear() {
        items.clear();
        batches.clear();
    }

    void Submit(const RenderItem& item) {
        items.push_back(item);
    }

    void BuildBatches() {
        batches.clear();

        for (const auto& item : items) {
            Shader* shader = item.material->asset->shader.get();

            BatchKey key{
                shader,
                item.mesh,
                item.submesh,
                item.material->asset.get()
            };

            auto& batch = batches[key];
            batch.key = key;
            batch.instances.push_back(item.model);
        }
    }

    const auto& GetBatches() const { return batches; }

private:
    std::vector<RenderItem> items;

    std::unordered_map< BatchKey, RenderBatch, BatchKeyHash > batches;
};

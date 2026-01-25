#pragma once
#include <typeindex>

using Entity = uint32_t;

struct ECSBridge {
    static void AddComponent(
        void* scene,
        Entity entity,
        std::type_index type
    );
};

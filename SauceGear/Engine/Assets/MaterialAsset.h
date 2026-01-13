#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include "IAsset.h"
#include "../Materials/MaterialBase.h"
#include "../Instancing/MaterialInstance.h"
#include "../Serializer/MaterialSerializer.h"

class MaterialAsset : public IAsset {
public: 
    std::string name;   //baseName
    std::shared_ptr<MaterialBase> base;
    std::unordered_map<std::string, MaterialInstance::Value> defaults;

    void Reload() override {
        auto fresh = MaterialSerializer::Load(path);
        base = fresh->base;
        defaults = std::move(fresh->defaults);
        lastWrite = std::filesystem::last_write_time(path);
    }   
    
    MaterialAsset() = default;

    explicit MaterialAsset(const std::shared_ptr<MaterialBase>& base) : base(base) {};

    static inline std::shared_ptr<MaterialInstance> Instantiate(const std::shared_ptr<MaterialAsset>& self) {
        return std::make_shared<MaterialInstance>(self);
    }   
};

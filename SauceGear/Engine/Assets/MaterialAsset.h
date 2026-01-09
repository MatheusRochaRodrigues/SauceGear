#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include "IAsset.h"
#include "../Materials/MaterialBase.h"
#include "../Instancing/MaterialInstance.h"
#include "../Serializer/MaterialSerializer.h"

class MaterialAsset : public IAsset, std::enable_shared_from_this<MaterialAsset> {
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
     

    std::shared_ptr<MaterialInstance> Instantiate() {
        return std::make_shared<MaterialInstance>(
            shared_from_this()
        );
    }


    /*std::shared_ptr<MaterialInstance> Instantiate() const { 
        return std::make_shared<MaterialInstance>( std::make_shared<MaterialAsset>(*this));
    }*/

};

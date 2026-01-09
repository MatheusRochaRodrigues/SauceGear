#pragma once  
#include <unordered_map>
#include <memory>
#include <string>
#include "MaterialBase.h"
#include "../Core/Log.h"

class MaterialLibrary {
public:
    static void Register(const std::string& name, std::shared_ptr<MaterialBase> base) {
        Get()[name] = base;
    }

    static std::shared_ptr<MaterialBase> Get(const std::string& name) {
        auto& map = Get();
        auto it = map.find(name);
        if (it == map.end()) {
            std::cout << "Material - " << name << std::endl;
            LOG_ERROR("MaterialBase '{}' não registrado", name);
            return nullptr;
        }
        return it->second;
    }

    static void InitMaterials();

private:
    static std::unordered_map<std::string, std::shared_ptr<MaterialBase>>& Get() {
        static std::unordered_map<std::string, std::shared_ptr<MaterialBase>> map;
        return map;
    }

};

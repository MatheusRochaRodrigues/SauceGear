//#pragma once
//#include <string>
//#include <memory>
//#include "../Graphics/Shader.h"
//#include "../Assets/IAsset.h"
//
//class ShaderAsset : public IAsset {
//public:
//    std::string path;
//    std::shared_ptr<Shader> shader;
//
//    static std::shared_ptr<ShaderAsset> Load(const std::string& path) {
//        auto asset = std::make_shared<ShaderAsset>();
//        asset->path = path;
//        asset->shader = std::make_shared<Shader>(path);
//        asset->lastWrite = std::filesystem::last_write_time(path);
//        return asset;
//    }
//
//    void Reload() override {
//        //shader->Reload();
//        lastWrite = std::filesystem::last_write_time(path);
//    }
//};

#pragma once 
#include <unordered_map>
#include <iostream>

class AssetDatabase {
public:
    template <typename T>
    static std::shared_ptr<T> Load(const std::string& path);

    template <typename T>
    static void Unload(const std::string& path);

private:
    static std::unordered_map<std::string, std::weak_ptr<void>> assets;
};

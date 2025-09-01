#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <cstddef>

struct FieldInfo {
    std::string name;
    std::type_index type;
    size_t offset;
};

struct TypeInfo {
    std::string name;
    std::vector<FieldInfo> fields;
};

class ReflectionRegistry {
public:
    static ReflectionRegistry& Get() {
        static ReflectionRegistry instance;
        return instance;
    }

    void RegisterType(const std::string& name, TypeInfo&& type) {
        types[name] = std::move(type);
    }

    TypeInfo* Get(const std::string& name) {
        auto it = types.find(name);
        return (it != types.end()) ? &it->second : nullptr;
    }

private:
    std::unordered_map<std::string, TypeInfo> types;
};

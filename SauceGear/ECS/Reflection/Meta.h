#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <cstddef>
#include <functional>

enum class FieldKind {
    Value,    // campo normal
    Header,   // header visual
    Space     // espaço vertical
};

struct FieldInfo {
    std::string name;
    std::type_index type{ typeid(void) }; // inicializa com typeid(void)
    size_t offset = 0; 

    FieldKind kind = FieldKind::Value; // default é campo normal
};

struct TypeInfo {
    std::string name;
    std::vector<FieldInfo> fields;
    std::type_index typeIndex = typeid(void); // novo: armazena type_index real

    // em TypeInfo:
    std::function<void(void* instance)> onEdited = nullptr; 
};

class ReflectionRegistry {
public:
    static ReflectionRegistry& Get() {
        static ReflectionRegistry instance;
        return instance;
    }

    // Registra o TypeInfo e guarda também type_index
    void RegisterType(const std::string& name, TypeInfo& type, std::type_index typeIndex) {        //TypeInfo&& type RValue
        type.typeIndex = typeIndex;
        types[name] = type;                                                                        //types[name] = std::move(type);
        typesByIndex[typeIndex] = &types[name];
    }

    TypeInfo* Get(const std::string& name) {
        auto it = types.find(name);
        return (it != types.end()) ? &it->second : nullptr;
    }


    TypeInfo* Get(std::type_index idx) {
        auto it = typesByIndex.find(idx);
        return (it != typesByIndex.end()) ? it->second : nullptr;
    }

private:
    std::unordered_map<std::string, TypeInfo> types; // lookup por nome
    std::unordered_map<std::type_index, TypeInfo*> typesByIndex; // lookup por type_index
};

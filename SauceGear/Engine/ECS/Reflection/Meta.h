#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <cstddef>
#include <functional>
//#include "../../Scene/SceneECS.h"

//class SceneECS;
using Entity = uint32_t; // ou forward do seu Entity


enum class FieldKind {
    Value,    // campo normal
    Header,   // header visual
    Space,     // espaço vertical

    Vector
};


struct EnumInfo {
    std::string name;

    struct Value {
        std::string label;
        int value;
    };

    std::vector<Value> values;
};
enum class EditorWidget {
    Default,
    EnumCombo,
    EnumRadio,
    EnumButtons,
    EnumFlags
};




struct FieldInfo {
    std::string name;
    std::type_index type{ typeid(void) }; // inicializa com typeid(void)
    size_t offset = 0; 

    FieldKind kind = FieldKind::Value; // default é campo normal


    std::type_index elementType{ typeid(void) }; // tipo interno (vector)
    // Interface genérica para containers
    size_t(*getSize)(void*) = nullptr;
    void* (*getElement)(void*, size_t) = nullptr;
    void   (*resize)(void*, size_t) = nullptr;


    // Enum
    EnumInfo* enumInfo = nullptr;
    EditorWidget widget = EditorWidget::Default;
};

struct TypeInfo {
    std::string name;
    std::vector<FieldInfo> fields;
    std::type_index typeIndex = typeid(void); // novo: armazena type_index real

    // em TypeInfo:
    std::function<void(void* instance)> onEdited = nullptr; 

    //AddComponent   -   factory ECS
    //std::function<void(SceneECS&, Entity)> Add;       //* > & ?
    void (*Add)(void* scene, Entity) = nullptr;                 //equivalent há = std::function<void(void*, Entity)> Add;

    bool removable = true;
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

    std::unordered_map<std::string, TypeInfo>& GetAll() {           //const std::unordered_map<std::string, TypeInfo>& GetAll() const
        return types;
    }

private:
    std::unordered_map<std::string, TypeInfo> types; // lookup por nome
    std::unordered_map<std::type_index, TypeInfo*> typesByIndex; // lookup por type_index
};



/*

EXTRA (nível Unreal / Unity)

Você pode evoluir isso depois para:

enum class ComponentFlags {
    None        = 0,
    NotRemovable= 1 << 0,
    Hidden      = 1 << 1,
    RuntimeOnly = 1 << 2
};

*/
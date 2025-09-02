#pragma once
#include "Meta.h"
#include <type_traits>
#include <typeindex>
//#include <typeinfo>

// Macros para registro automático
#define REFLECT_CLASS(CLASS) \
    static TypeInfo& GetTypeInfo() { \
        static TypeInfo typeInfo{#CLASS}; \
        return typeInfo; \
    } \
    struct __ReflectionHelper { \
        __ReflectionHelper() { \
            CLASS::RegisterFields(); \
            ReflectionRegistry::Get().RegisterType(#CLASS, CLASS::GetTypeInfo(), typeid(CLASS)); \
        } \
    }; \
    static inline __ReflectionHelper __reflectionHelperInstance; \
    using __CurrentClass = CLASS; \
    static void RegisterFields()

#define REFLECT_FIELD(FIELD) \
    { \
        FieldInfo f; \
        f.name = #FIELD; \
        f.type = std::type_index(typeid(decltype(__CurrentClass::FIELD))); \
        f.offset = offsetof(__CurrentClass, FIELD); \
        GetTypeInfo().fields.push_back(f); \
    }

#define REFLECT_HEADER(LABEL) \
    GetTypeInfo().fields.push_back({ LABEL, typeid(void), 0, FieldKind::Header });

#define REFLECT_SPACE() \
    GetTypeInfo().fields.push_back({ "", typeid(void), 0, FieldKind::Space });

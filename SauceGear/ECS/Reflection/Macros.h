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
    static void RegisterFields()

#define REFLECT_FIELD(CLASS, FIELD) \
    { \
        FieldInfo f; \
        f.name = #FIELD; \
        f.type = std::type_index(typeid(decltype(CLASS::FIELD))); \
        f.offset = offsetof(CLASS, FIELD); \
        GetTypeInfo().fields.push_back(f); \
    }

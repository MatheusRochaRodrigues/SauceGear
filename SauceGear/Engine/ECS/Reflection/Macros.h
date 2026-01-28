#pragma once
#include "Meta.h"
#include <type_traits>
#include <typeindex>
//#include <typeinfo>
#include "../../Scene/ECSBridge.h" 

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



#define REFLECT_ON_EDITED(BODY) \
    GetTypeInfo().onEdited = [](void* instance) { \
        using T = __CurrentClass; \
        T* obj = static_cast<T*>(instance); \
        BODY \
    };





#define REFLECT_VECTOR(FIELD, ELEMENT_TYPE)                               \
{                                                                         \
    FieldInfo f;                                                          \
    f.name = #FIELD;                                                      \
    f.kind = FieldKind::Vector;                                           \
    f.type = typeid(std::vector<ELEMENT_TYPE>);                           \
    f.elementType = typeid(ELEMENT_TYPE);                                 \
    f.offset = offsetof(__CurrentClass, FIELD);                           \
                                                                          \
    f.getSize = [](void* fieldPtr) -> size_t {                             \
        auto& v = *static_cast<std::vector<ELEMENT_TYPE>*>(fieldPtr);     \
        return v.size();                                                  \
    };                                                                    \
                                                                          \
    f.getElement = [](void* fieldPtr, size_t i) -> void* {                \
        auto& v = *static_cast<std::vector<ELEMENT_TYPE>*>(fieldPtr);     \
        return &v[i];                                                     \
    };                                                                    \
                                                                          \
    f.resize = [](void* fieldPtr, size_t sz) {                             \
        auto& v = *static_cast<std::vector<ELEMENT_TYPE>*>(fieldPtr);     \
        v.resize(sz);                                                     \
    };                                                                    \
                                                                          \
    GetTypeInfo().fields.push_back(f);                                    \
}





#define BEGIN_ENUM_INFO(ENUM) \
    EnumInfo ENUM##_EnumInfo{ #ENUM, {

#define ENUM_VALUE(ENUM, VALUE) \
    { #VALUE, (int)ENUM::VALUE },

#define END_ENUM_INFO() } };


#define REFLECT_ENUM_FIELD(FIELD, ENUM, WIDGET) \
{ \
    FieldInfo f; \
    f.name = #FIELD; \
    f.type = typeid(ENUM); \
    f.offset = offsetof(__CurrentClass, FIELD); \
    f.enumInfo = &ENUM##_EnumInfo; \
    f.widget = WIDGET; \
    GetTypeInfo().fields.push_back(f); \
}




//#define REFLECT_ADD_COMPONENT() \
//    GetTypeInfo().Add = [](SceneECS& scene, Entity e) { \
//        scene.AddComponent<__CurrentClass>(e); \
//    };

#define REFLECT_ADD_COMPONENT()                                    \
    GetTypeInfo().Add = [](void* scene, Entity e) {               \
        ECSBridge::AddComponent(scene, e, GetTypeInfo().typeIndex); \
    };



#define REFLECT_NOT_REMOVABLE() \
    GetTypeInfo().removable = false;


#define REFLECT_FIELD_COLOR(FIELD) \
{ \
    FieldInfo f; \
    f.name = #FIELD; \
    f.type = std::type_index(typeid(decltype(__CurrentClass::FIELD))); \
    f.offset = offsetof(__CurrentClass, FIELD); \
    f.widget = EditorWidget::Color; \
    GetTypeInfo().fields.push_back(f); \
}


#define REFLECT_FLOAT_SLIDER(FIELD, MIN, MAX) \
{ \
    FieldInfo f; \
    f.name = #FIELD; \
    f.type = typeid(float); \
    f.offset = offsetof(__CurrentClass, FIELD); \
    f.widget = EditorWidget::SliderFloat; \
    f.min = MIN; \
    f.max = MAX; \
    GetTypeInfo().fields.push_back(f); \
}

#define REFLECT_INT_SLIDER(FIELD, MIN, MAX) \
{ \
    FieldInfo f; \
    f.name = #FIELD; \
    f.type = typeid(int); \
    f.offset = offsetof(__CurrentClass, FIELD); \
    f.widget = EditorWidget::SliderInt; \
    f.min = (float)(MIN); \
    f.max = (float)(MAX); \
    GetTypeInfo().fields.push_back(f); \
}

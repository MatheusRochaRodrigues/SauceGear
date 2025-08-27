#pragma once
#include <stdint.h>

#ifdef _WIN32
#define SCRIPT_API extern "C" __declspec(dllexport)
#else
#define SCRIPT_API extern "C"
#endif

// Forward só por id; acesso ao mundo via funções de ponteiro passadas pela engine (evita STL na ABI)
using EntityId = uint32_t;

// Versão do ABI para checagem (altere se quebrar a interface)
static const uint32_t SCRIPT_ABI_VERSION = 1;

// Mini vtable C para o ScriptBehaviour
struct ScriptVTable {
    void (*create)(void* self);
    void (*update)(void* self, float dt);
    void (*destroy)(void* self);
};

// Objeto “opaco” que vive na DLL
struct ScriptInstance {
    void* self;     // ponteiro para a classe real, alocada na DLL
    ScriptVTable     vtbl;     // ponteiros de função para ciclo de vida
    EntityId         entity;   // id ECS
};

// Funções exigidas pela engine (nomes fixos):
// Factory - cria uma instância para um entity
SCRIPT_API ScriptInstance* CreateScriptInstance(EntityId entity);
// Destrói a instância
SCRIPT_API void DestroyScriptInstance(ScriptInstance* inst);
// Versão/identidade do plugin
SCRIPT_API uint32_t GetScriptABIVersion();
SCRIPT_API const char* GetScriptName();

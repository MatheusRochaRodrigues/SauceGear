#pragma once
#include <string>
#include <vector>
#include "ScriptBehaviour.h" // para VariableInfo

#define EXPORT_SCRIPT(ClassName) \
extern "C" __declspec(dllexport) ScriptBehaviour* CreateScript() { \
    return new ClassName(); \
}


#define EXPOSE_INT(name)    exposedVars.push_back({#name, &name, VariableInfo::Int})
#define EXPOSE_FLOAT(name)  exposedVars.push_back({#name, &name, VariableInfo::Float})
#define EXPOSE_VEC3(name)   exposedVars.push_back({#name, &name, VariableInfo::Vec3})

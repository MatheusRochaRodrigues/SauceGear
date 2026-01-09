#pragma once
#include <unordered_map>
#include <string>
#include "../../Graphics/Shader.h"

class ShaderLibrary {
public:
    // Inicializa todos os shaders (chamar no startup da engine)
    static void Init();

    // Acesso global e seguro
    static Shader& Get(const std::string& name);

    // Opcional: checagem
    static bool Exists(const std::string& name);

private:
    static std::unordered_map<std::string, Shader> shaders;
};

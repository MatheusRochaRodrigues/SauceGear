#pragma once
#include "ShaderBase.h"
#include <string>
#include <vector>
#include <utility> 

using namespace std;

class Shader : public ShaderBase {
public:
    string vertexFile;
    string geometryFile;
    string fragmentFile;

    Shader() = default;
    Shader(string vertex, string fragment, const std::vector<std::pair<std::string, int>>& defines = {}, bool debug = false);
    Shader(string vertex, string geometry, string fragment, const std::vector<std::pair<std::string, int>>& defines = {}, bool debug = false);

    void ReloadWithDefines(const std::vector<std::pair<std::string, int>>& defines);

    std::string path = "";
};

#pragma once
#include <string>
#include <filesystem>
#include <regex> 
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <vector>
#include <unordered_map> 
#include <stdexcept> 

class ShaderPreprocessor {
public:
    static std::string get_file_contents(const char* filename)
    {
        std::ifstream in(filename, std::ios::binary);
        if (!in)
            throw std::runtime_error(std::string("\n<Error> To open archive Shader : ") + filename);
        //throw(errno);

        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return contents;
    }

    // Expande includes e injeta defines
    inline static std::string ProcessFile(const std::filesystem::path& filePath,
        const std::vector<std::pair<std::string, int>>& defines)
    {
        std::string source = ReadFile(filePath);
        source = ProcessSource(source, filePath.parent_path(), 0, defines);
        source = UpdateDefine(source, defines);
        return source;
    }

    inline static void ClearCache() {
        includeCache.clear();
    }

private:
    inline static const std::filesystem::path GLOBAL_SHADER_DIR = "resources/shaders/utils/";
    inline static std::unordered_map<std::string, std::string> includeCache;

    // Lê um arquivo e retorna string, com cache
    inline static std::string ReadFile(const std::filesystem::path& path) {
        std::string key = std::filesystem::absolute(path).string();

        auto it = includeCache.find(key);
        if (it != includeCache.end()) return it->second;

        if (!std::filesystem::exists(path))
            throw std::runtime_error("Arquivo não encontrado: " + path.string());

        std::ifstream file(path);
        if (!file.is_open())
            throw std::runtime_error("Erro ao abrir: " + path.string());

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string contents = buffer.str();
        includeCache[key] = contents;
        return contents;
    }

    // Carrega include relativo ou global
    inline static std::string LoadInclude(const std::string& includeFile,
        const std::filesystem::path& baseDir,
        bool isGlobal)
    {
        std::filesystem::path includePath;
        if (isGlobal) {
            includePath = GLOBAL_SHADER_DIR / includeFile;
            if (!std::filesystem::exists(includePath))
                throw std::runtime_error("Include global <" + includeFile + "> não encontrado em " + GLOBAL_SHADER_DIR.string());
        }
        else {
            includePath = baseDir / includeFile;
            if (!std::filesystem::exists(includePath))
                throw std::runtime_error("Include relativo \"" + includeFile + "\" não encontrado relativo a " + baseDir.string());
        }
        return ReadFile(includePath);
    }

    // Processa source expandindo includes e adicionando #line
    inline static std::string ProcessSource(const std::string& source,
        const std::filesystem::path& baseDir,
        int depth,
        const std::vector<std::pair<std::string, int>>& defines)
    {
        if (depth > 32)
            throw std::runtime_error("Include recursivo muito profundo (>32)");

        std::stringstream output;
        std::regex includeRegex(R"(#include\s*([<"])([^">]+)[">])");

        std::istringstream iss(source);
        std::string line;
        int lineNumber = 1;

        while (std::getline(iss, line)) {
            std::smatch match;
            if (std::regex_search(line, match, includeRegex)) {
                bool isGlobal = (match[1] == "<");
                std::string includeFile = match[2];

                std::string includeSource = LoadInclude(includeFile, baseDir, isGlobal);

                // #line para erros do compilador
                output << "#line 1 \"" << includeFile << "\"\n";
                output << ProcessSource(includeSource, isGlobal ? GLOBAL_SHADER_DIR : baseDir, depth + 1, defines);
                output << "#line " << (lineNumber + 1) << " \"" << baseDir.filename().string() << "\"\n";
            }
            else {
                output << line << "\n";
            }
            lineNumber++;
        }

        return output.str();
    }

    // Injeta defines no topo
    inline static std::string UpdateDefine(const std::string& shaderCode,
        const std::vector<std::pair<std::string, int>>& defines)
    {
        std::string code = shaderCode;
        for (auto& def : defines) {
            const std::string& name = def.first;
            int value = def.second;
            std::regex defineRegex("#define\\s+" + name + "\\s+\\d+");
            if (std::regex_search(code, defineRegex))
                code = std::regex_replace(code, defineRegex, "#define " + name + " " + std::to_string(value));
            else
                code = "#define " + name + " " + std::to_string(value) + "\n" + code;
        }
        return code;
    }
};





/*
//Legacy
// Read vertexFile and fragmentFile and store the strings

vertexFile = vPath.c_str();
fragmentFile = fPath.c_str();
try {
	std::string vertexCode = get_file_contents(vertexFile);
	std::string fragmentCode = get_file_contents(fragmentFile);

std::string get_file_contents(const char* filename);
// Reads a text file and outputs a string with everything in the text file

*/
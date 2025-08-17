//#pragma once
//#include "../Scene/Components/Material.h"
//#include <unordered_map>
//#include <string>
//
//unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);
//
//class AssetManager {
//public:
//    static std::unordered_map<std::string, Material*> materialCache;
//    static std::unordered_map<std::string, Texture*> textureCache;
//
//    static Material* LoadMaterial(const std::string& path) {
//        if (materialCache.find(path) != materialCache.end())
//            return materialCache[path];
//
//        Material* mat = new Material();
//        // aqui você carregaria textura, shader, etc...
//        materialCache[path] = mat;
//        return mat;
//    }
//
//    static Texture* LoadTexture(const std::string& path) {
//        if (textureCache.find(path) != textureCache.end())
//            return textureCache[path];
//
//        Texture* tex = new Texture();
//        tex->LoadFromFile(path.c_str());
//        textureCache[path] = tex;
//        return tex;
//    }
//
//    static void ReloadMaterial(const std::string& path) {
//        if (materialCache.find(path) != materialCache.end()) {
//            delete materialCache[path];
//            materialCache.erase(path);
//        }
//        LoadMaterial(path);
//    }
//
//    static void ReloadTexture(const std::string& path) {
//        if (textureCache.find(path) != textureCache.end()) {
//            delete textureCache[path];
//            textureCache.erase(path);
//        }
//        LoadTexture(path);
//    }
//};
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
//{
//    string filename = string(path);
//    filename = directory + '/' + filename;
//
//    unsigned int textureID;
//    glGenTextures(1, &textureID);
//
//    int width, height, nrComponents;
//    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
//    if (data)
//    {
//        GLenum format;
//        if (nrComponents == 1)
//            format = GL_RED;
//        else if (nrComponents == 3)
//            format = GL_RGB;
//        else if (nrComponents == 4)
//            format = GL_RGBA;
//
//        glBindTexture(GL_TEXTURE_2D, textureID);
//        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//        glGenerateMipmap(GL_TEXTURE_2D);
//
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//        stbi_image_free(data);
//    }
//    else
//    {
//        std::cout << "Texture failed to load at path: " << path << std::endl;
//        stbi_image_free(data);
//    }
//
//    return textureID;
//}

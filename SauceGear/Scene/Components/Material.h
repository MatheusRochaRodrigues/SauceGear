#pragma once
#include "../../Graphics/Texture.h"  
#include <unordered_map>
#include <string>

namespace MaterialDefaults {
    Texture* WhiteTexture();
}
 
struct Material {
    Shader* shader; 
    std::unordered_map<std::string, Texture*> textures; 
    //std::unordered_map<std::string, std::pair<unsigned int,Texture*>> textures; 

    std::unordered_map<std::string, float>    floatParams; 
    glm::vec3 albedoColor = glm::vec3(1.0f);

    Material(Shader* shader) : shader(shader) {} 
    Material() { 
        Shader* shader = new Shader("BlinnPhong/BaseLighting.vs", "BlinnPhong/BaseLighting.fs");
        this->shader = shader; 
    } 

    //void setData(unsigned int u, std::string s, Texture* t) { textures[s] = { u, t }; }
    //void setData(std::string s, float f) { floatParams[s] =  f ; }

    void Bind() const {
        shader->use();
        // Bind texturas
        int slot = 0;
        bool hasNormalMap = false;

        for (auto& [name, tex] : textures) {        //[name, tex]
            // fallback: se tex for nullptr, usa textura branca
            //if (!tex) tex = Texture::WhiteTexture(); // fallback
            //Texture* finalTex = tex ? tex : Texture::WhiteTexture();
            slot = tex->unit;

            glActiveTexture(GL_TEXTURE0 + slot);
            tex->Bind(); // tex->Bind() deve usar glBindTexture(...)
            shader->setInt(name, slot); // "diffuseMap" -> slot 0
            //slot++; 
            if (name == "Normal" && tex) { // se realmente tem normalMap carregado
                hasNormalMap = true;
            }
        }

        // seta uniform bool
        shader->setBool("op_Normal", hasNormalMap);

        // Bind valores float
        for (auto& [name, value] : floatParams) {
            shader->setFloat(name, value);
        }
    }

    void setLayer() const { 
        // Bind texturas
        int slot = 0;
        for (auto& [name, tex] : textures) {     
            slot = tex->unit;

            glActiveTexture(GL_TEXTURE0 + slot);
            tex->Bind(); // tex->Bind() deve usar glBindTexture(...)
            shader->setInt(name, slot); // "diffuseMap" -> slot 0
            slot++;
        }

        // Bind valores float
        for (auto& [name, value] : floatParams) {
            shader->setFloat(name, value);
        }
    }

    void Apply(Shader* shaderOverride) const {
        Shader* activeShader = shaderOverride ? shaderOverride : shader;
        if (!activeShader) return;

        activeShader->use();

        int unit = 0;
        for (const auto& [name, tex] : textures) {  
            glActiveTexture(GL_TEXTURE0 + unit);    //glBindTexture(GL_TEXTURE_2D, tex->ID); 
            //glActiveTexture(GL_TEXTURE0 + unit);    //glBindTexture(GL_TEXTURE_2D, tex->ID); 
            tex->Bind();
            activeShader->setInt(name.c_str(), unit);
            unit++;
        }

        // Bind valores float
        for (auto& [name, value] : floatParams) {
            shader->setFloat(name, value);
        }
    }

    /*void Apply(Shader* shaderOverride) const {
        Shader* activeShader = shaderOverride ? shaderOverride : shader;
        if (!activeShader) return;

        activeShader->use();

        for (size_t i = 0; i < textures.size(); ++i) {
            if (textures[i]) {
                std::string uniformName = "texture" + std::to_string(i);
                activeShader->setInt(uniformName.c_str(), static_cast<int>(i));
                glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
                glBindTexture(GL_TEXTURE_2D, textures[i]->ID);
            }
        }
    }*/  

};
    //static Shader* DefaultMaterial;


// Fallbacks e registros
namespace MaterialDefaults {
    static Texture* whiteTex = nullptr;

    static Texture* TextureColor(uint8_t x, uint8_t y, uint8_t z) { 
        uint8_t whitePixel[4] = { x, y, z, 255 };
        Texture* tex = new Texture();
        tex->Texture::CreateFromMemory(whitePixel, 1, 1, GL_RGBA); // sua função para gerar textura a partir de memória 
        return tex;
    }

    //Create white texture default (1x1 resolution)
    inline static Texture* WhiteTexture() {
        if (!whiteTex) {
            uint8_t whitePixel[4] = { 255, 255, 255, 255 };
            whiteTex = new Texture();
            whiteTex->Texture::CreateFromMemory(whitePixel, 1, 1, GL_RGBA); // sua função para gerar textura a partir de memória 
        }
        return whiteTex;
    }

    inline static Material* Get() { 
        Material* defaultMat = new Material();

        defaultMat->textures["albedoMap"]    = WhiteTexture();
        /*defaultMat->textures["specularMap"]  = { 1,  WhiteTexture() };
        defaultMat->textures["normalMap"]    = { 2,  WhiteTexture() };
        defaultMat->textures["heightMap"]    = { 3,  WhiteTexture() };*/
        defaultMat->floatParams["roughness"] = 0.5f;
        defaultMat->floatParams["metallic"]  = 0.1f;
          
        return defaultMat;
    }



    //pbr
    //inline static Texture* whiteTex = nullptr;
    //inline static Texture* normalTex = nullptr;
    //inline static Texture* roughTex = nullptr;

    //static Texture* WhiteTexture() {
    //    if (!whiteTex) {
    //        uint8_t pixel[4] = { 255, 255, 255, 255 };
    //        whiteTex = new Texture();
    //        whiteTex->CreateFromMemory(pixel, 1, 1, GL_RGBA);
    //    }
    //    return whiteTex;
    //}

    //static Texture* NormalTexture() {
    //    if (!normalTex) {
    //        uint8_t pixel[4] = { 128, 128, 255, 255 }; // normal padrão (0,0,1)
    //        normalTex = new Texture();
    //        normalTex->CreateFromMemory(pixel, 1, 1, GL_RGBA);
    //    }
    //    return normalTex;
    //}

    //static Texture* RoughnessTexture() {
    //    if (!roughTex) {
    //        uint8_t pixel[4] = { 128, 128, 128, 255 }; // roughness médio
    //        roughTex = new Texture();
    //        roughTex->CreateFromMemory(pixel, 1, 1, GL_RGBA);
    //    }
    //    return roughTex;
    //}

    //static Material* Default() {
    //    Shader* shader = new Shader("PBR/Base.vs", "PBR/Base.fs");
    //    Material* mat = new Material(shader);
    //    mat->textures["albedoMap"] = MaterialDefaults::WhiteTexture();
    //    mat->textures["normalMap"] = MaterialDefaults::NormalTexture();
    //    mat->textures["roughnessMap"] = MaterialDefaults::RoughnessTexture();
    //    mat->floatParams["metallic"] = 0.0f;
    //    mat->floatParams["roughness"] = 0.5f;
    //    return mat;
    //}

}
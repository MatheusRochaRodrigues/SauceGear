#pragma once 
#include <string>
#include <vector>
#include "../Graphics/Shader.h"
#include <stb/stb_image.h> 

class Texture {
public:
    unsigned int ID = 0;
    std::string type;         // Tipo: diffuse, specular, etc.
    std::string path;         // Caminho original (se aplicável)
    GLenum format = GL_RGB;   // Formato de leitura
    GLuint unit = 0;          // Unidade de textura vinculada
    bool sucess = false;

    // Construtores
    Texture() = default;

    // Textura 2D a partir de imagem
    Texture(const char* path, const std::string& type, GLuint unit, bool useSRGB = false);

    //Texture(const Texture& other) {
    //    // Copia tipo, path, etc
    //    this->type = other.type;
    //    this->path = other.path;
    //    this->format = other.format;
    //    this->unit = other.unit;

    //    // Copia a textura real da GPU (cria uma nova e copia os dados)
    //    glGenTextures(1, &this->ID);
    //    glBindTexture(GL_TEXTURE_2D, other.ID);

    //    // Isso apenas reutiliza os dados do texture atual
    //    // Para deep copy real: você precisaria pegar os dados via glGetTexImage (OpenGL >= 4.5) ou salvar e recarregar
    //    // Aqui vai só um placeholder:
    //    // glGetTexImage(...); -> requires a lot of boilerplate

    //    // Alternativa: em vez de duplicar a textura, **compartilhe o ponteiro** (mais leve e seguro)
    //}  

    // Textura para framebuffer (2D ou multisample)
    Texture(unsigned int width, unsigned int height,
        GLenum internalFormat = GL_RGB,
        bool MultiSamples = 0,
        GLenum format = GL_RGB,
        unsigned int samples = 0);

    // Criar cubemap a partir de faces
    Texture(const std::vector<std::string>& faces); // faces[6]

    // Criar um cubemap vazio (ex: shadow map directional light)
    void GenerateCubeMap(unsigned int width, unsigned int height); 


    // Render target para back buffer (ex: pós-processamento)
    void GenerateRenderTarget(unsigned int width, unsigned int height,
        GLenum internalFormat,
        GLint filter = GL_NEAREST,
        GLenum format = -1    /*GLenum format = GL_RGB*/);

    // Bind à unidade de textura para uso em shaders
    void BindToUnit(Shader& shader, const char* uniformName, GLuint textureUnit);



    void Bind(int slot = -1) const;
    void Unbind() const;
    void Delete();

    void SetUnit(Shader& shader, const char* uniform, GLuint unit);


    unsigned int LoadFromFile(const std::string& filename, bool gamma = false);
    Texture* WhiteTexture();
    void CreateFromMemory(uint8_t* data, int width, int height, GLenum format = GL_RGBA);
    
    static void attechment() {

    }
private:
    void LoadFromFile(const char* path, bool useSRGB);
    void LoadCubeMap(const std::vector<std::string>& faces);

};

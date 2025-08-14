#ifndef MODEL_H
#define MODEL_H

#include <stb/stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Graphics/Mesh.h"

using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model
{
public:
    // model data 
    //std::vector<Material*> materials; // adicionamos aqui
    //std::unordered_map<Mesh*, Material*> meshMaterials; // mapeia cada mesh ao seu material

    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>    meshes; //subMeshes
    string directory;
    bool gammaCorrection;

    Model();

    // constructor, expects a filepath to a 3D model.
    Model(string const& path, bool gamma = false);


    // draws the model, and thus all its meshes
    void Draw();        //Shader& shader

    void AddMesh(const Mesh& mesh) {
        meshes.push_back(mesh);
    }

    const std::vector<Mesh>& GetMeshes() const { return meshes; }
private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path);     //LoadMesh

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
    Material* CreateMaterialFrom(aiMaterial* aiMat, const std::string& directory);
    //Material* CreateMaterialFrom(aiMaterial* aiMat, const std::string& directory, Shader* defaultShader);

    std::unordered_map<unsigned int, Material*> materialCache;

};


#endif
#ifndef MODEL_H
#define MODEL_H

#include <stb/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Graphics/Mesh.h" 
#include "AssetManager.h"

#include "DefineMaterials/TextureCache.h"
#include "DataBase/AssetDatabase.h"

//using matCache = std::unordered_map<unsigned int, std::shared_ptr<MaterialInstance>>;         matCache materialCache;
static std::vector<Texture> textures_loaded;

class ModelLoader {
public:
    static Mesh* LoadModel(string const& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices
        );

        if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return nullptr;
        }

        //auto* root = new Mesh();
        //root->name = scene->mRootNode->mName.C_Str();
        //root->directory = path.substr(0, path.find_last_of("/\\"));
         
        Mesh* mesh = ProcessNode(scene->mRootNode, scene, path.substr(0, path.find_last_of("/\\")));     //ProcessNode(scene->mRootNode, scene, root, materialCache);
        mesh->name = scene->mRootNode->mName.C_Str();

        return mesh;  
    } 

private:
    static glm::vec3 AiToVec3(const aiVector3D& v)    {  return  { v.x, v.y, v.z }; }
    static glm::quat AiToQuat(const aiQuaternion& q) { return { q.w, q.x, q.y, q.z }; }

    // Converte Assimp node -> Mesh (agregando TODAS as aiMeshes desse node como submeshes)
    static Mesh* ProcessNode(aiNode* node, const aiScene* scene, string path) {
        // Cria um Mesh “container” pra este node (mesmo se não tiver geometria, mantemos o nó pra hierarquia)
        auto* meshNode = new Mesh();
        meshNode->name = node->mName.C_Str();
        meshNode->directory = path;                        //meshNode->directory = parent->directory;


        std::string modelName = path.substr(path.find_last_of("/\\") + 1); // nome do arquivo com extensão
        auto pos = modelName.find_last_of('.'); if (pos != std::string::npos) modelName = modelName.substr(0, pos);

        // Se o node tem meshes, agregamos todas em um só Mesh (com submeshes)
        if (node->mNumMeshes > 0) {
            std::vector<Vertex>  vertices;
            std::vector<uint32_t> indices;
            std::vector<SubMesh> submeshes;

            // Extrai TRS do nó
            /*aiVector3D scaling, position;
            aiQuaternion rotation;
            node->mTransformation.Decompose(scaling, rotation, position); */

            /*parentMesh->localPosition = AiToVec3(position);
            parentMesh->localRotation = AiToQuat(rotation);
            parentMesh->localScale    = AiToVec3(scaling);*/

            for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
                aiMesh* imesh = scene->mMeshes[node->mMeshes[i]];
                const unsigned int matIndex = imesh->mMaterialIndex; 
                const uint32_t baseVertex = static_cast<uint32_t>(vertices.size());

                // vertices
                vertices.reserve(vertices.size() + imesh->mNumVertices);
                for (unsigned int v = 0; v < imesh->mNumVertices; ++v) {
                    Vertex vert{};
                    vert.Position = { imesh->mVertices[v].x, imesh->mVertices[v].y, imesh->mVertices[v].z };
                    vert.Normal = imesh->HasNormals() ?
                        glm::vec3(imesh->mNormals[v].x, imesh->mNormals[v].y, imesh->mNormals[v].z)
                        : glm::vec3(0, 1, 0);

                    if (imesh->mTextureCoords[0]) {
                        vert.TexCoords = { imesh->mTextureCoords[0][v].x, imesh->mTextureCoords[0][v].y };
                        if (imesh->mTangents) {
                            vert.Tangent = { imesh->mTangents[v].x, imesh->mTangents[v].y, imesh->mTangents[v].z };
                            vert.Bitangent = { imesh->mBitangents[v].x, imesh->mBitangents[v].y, imesh->mBitangents[v].z };
                        }
                    }
                    else {
                        vert.TexCoords = { 0.0f, 0.0f };
                    } 
                    for (int j = 0; j < MAX_BONE_INFLUENCE; ++j) {
                        vert.m_BoneIDs[j] = 0;
                        vert.m_Weights[j] = 0.0f;
                    } 
                    vertices.push_back(vert);
                }

                // indices (um submesh por aiMesh)
                const uint32_t indexOffset = static_cast<uint32_t>(indices.size());
                for (unsigned int f = 0; f < imesh->mNumFaces; ++f) {
                    const aiFace& face = imesh->mFaces[f];
                    for (unsigned int j = 0; j < face.mNumIndices; ++j)
                        indices.push_back(baseVertex + face.mIndices[j]);
                }
                const uint32_t indexCount = static_cast<uint32_t>(indices.size()) - indexOffset;
                 
                // material (com cache por materialIndex)  
                auto mat = CreateMaterialInstanceFrom(scene->mMaterials[matIndex], meshNode->directory, modelName); 
                submeshes.push_back(SubMesh{ indexOffset, indexCount, mat });
            } 
            meshNode->Set(vertices, indices, submeshes); // chama setupMesh()
        }  
              
        // Recursão pros filhos Assimp
        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            // Anexa este node ao pai
            meshNode->AddChild (ProcessNode(node->mChildren[i], scene, path));
        }

        return meshNode;
    }
    //static Mesh* ProcessMesh(aiNode* node, aiMesh* aimesh, const aiScene* scene, const std::string& directory, matCache& materialCache) { }

    static std::shared_ptr<MaterialInstance> CreateMaterialInstanceFrom(aiMaterial* aiMat,
        const std::string& directory,
        const std::string& modelName)
    { 
        // Chave única: modelName + materialName
        std::string key = AssetDatabaseUtils::MakeMaterialKey(modelName, aiMat->GetName().C_Str());

        // Se já existe, retorna
        auto existing = AssetDatabase::Load<MaterialInstance>(key, [&]() -> std::shared_ptr<MaterialInstance> {
            // Base do material: PBR ou outro custom
            auto baseMat = std::make_shared<PBRMaterial>();          //std::shared_ptr<MaterialBase> baseMat = std::make_shared<PBRMaterial>();
            auto instance = std::make_shared<MaterialInstance>(baseMat);

            // Load cores / fallback
            aiColor3D color(1.f, 1.f, 1.f); 

            // Textura difusa
            aiString texPath;
            if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
                std::string fullPath = directory + "/" + texPath.C_Str();
                instance->SetTexture("Albedo", TextureCache::Get().Load(fullPath)).unit = 1; 
            }

            // fallback de metallic e roughness
            instance->SetFallbackFloat("Metallic", 0.1f).unit = 2;
            instance->SetFallbackFloat("Roughness", 0.5f). unit = 3;

            return instance;
        });

        return existing;
    }



    //static std::shared_ptr<MaterialInstance> CreateMaterialInstanceFrom(aiMaterial* aiMat, const std::string& directory) {
    //    std::string matName = aiMat->GetName().C_Str();
    //    auto mat = AssetDatabase::Load<MaterialInstance>(matName);

    //    // Base do material: PBR ou outro material custom
    //    auto baseMat = std::make_shared<PBRMaterial>(); 
    //    auto instance = std::make_shared<MaterialInstance>(baseMat);

    //    // Load textures / cores
    //    aiColor3D color(1.f, 1.f, 1.f);
    //    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color))  
    //        instance->SetFallbackColor("Albedo", glm::vec3(color.r, color.g, color.b));
    //      
    //    aiString texPath;
    //    if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
    //        std::string fullPath = directory + "/" + texPath.C_Str(); 
    //        instance->SetTexture("Albedo", TextureCache::Get().Load(fullPath));
    //    }

    //    // fallback de metallic e roughness
    //    instance->SetFallbackFloat("Metallic", 0.1f);
    //    instance->SetFallbackFloat("Roughness", 0.5f);

    //    return instance;
    //}
    
    /*
    static std::shared_ptr<MaterialInstance> CreateMaterialFrom(aiMaterial* aiMat, const std::string& directory) {
        // 1. Criar o MaterialBase (ou uma subclasse se quiser special bindings)
        auto baseMat = std::make_shared<MaterialBase>(ShaderManager::GetShader("PBR"));
        baseMat->DefineParameters(); // define defaults (albedo, roughness, metallic, etc)

        // 2. Criar a MaterialInstance a partir do base
        auto instance = std::make_shared<MaterialInstance>(baseMat);

        // 3. Carregar texturas do aiMaterial
        auto LoadTexture = [&](aiTextureType type, const std::string& uniformName, const glm::vec3& fallbackColor) {
            aiString str;
            if (aiMat->GetTexture(type, 0, &str) == AI_SUCCESS) {
                const std::string fullPath = directory + "/" + str.C_Str();
                auto tex = std::make_shared<Texture>();
                tex->LoadFromFile(fullPath.c_str());
                instance->SetTexture(uniformName, tex);
            }
            else {
                // fallback: cor
                instance->SetFallbackColor(uniformName, fallbackColor);
            }
            };

        // Albedo
        aiColor3D diffuse(1.f, 1.f, 1.f);
        aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        LoadTexture(aiTextureType_DIFFUSE, "albedo", glm::vec3(diffuse.r, diffuse.g, diffuse.b));

        // Roughness (float fallback)
        float roughness = 0.5f;
        instance->SetFallbackFloat("roughness", roughness);

        // Metallic (float fallback)
        float metallic = 0.1f;
        instance->SetFallbackFloat("metallic", metallic);

        // Normal map (fallback branco)
        LoadTexture(aiTextureType_HEIGHT, "normal", glm::vec3(1.f, 1.f, 1.f));

        return instance;
    }
    */


    /*
    static Material* CreateMaterialFrom(aiMaterial* aiMat, const std::string& directory) {
        auto* material = new Material();

        unsigned int unitTex = 0;
        auto LoadSingleTexture = [&](aiTextureType type, const std::string& uniformName, Texture* fallback) {
            aiString str;
            if (aiMat->GetTexture(type, 0, &str) == AI_SUCCESS) {
                const std::string fullPath = directory + "/" + str.C_Str();

                // cache local simples por caminho
                for (const auto& tex : textures_loaded) {
                    if (tex.path == str.C_Str()) {
                        material->textures[uniformName] = new Texture(tex) ;
                        return;
                    }
                } 
                auto* tex = new Texture();
                tex->unit = unitTex++;
                tex->LoadFromFile(fullPath.c_str());
                tex->path = str.C_Str();
                material->textures[uniformName] = tex;
                textures_loaded.push_back(*tex);
            }
            else {
                // fallback por cor difusa
                aiColor3D color(1.f, 1.f, 1.f);
                if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
                    material->albedoColor = glm::vec3(color.r, color.g, color.b);
                }
                material->textures[uniformName] = { 
                    MaterialDefaults::TextureColor(
                    static_cast<uint8_t>(material->albedoColor.r * 255.0f),
                    static_cast<uint8_t>(material->albedoColor.g * 255.0f),
                    static_cast<uint8_t>(material->albedoColor.b * 255.0f)
                ) };
            }
        };

        LoadSingleTexture(aiTextureType_DIFFUSE, "Albedo", MaterialDefaults::WhiteTexture());
        //LoadSingleTexture(aiTextureType_SPECULAR, "specularMap", MaterialDefaults::WhiteTexture());
        LoadSingleTexture(aiTextureType_HEIGHT, "Normal", MaterialDefaults::WhiteTexture());
        //LoadSingleTexture(aiTextureType_AMBIENT,  "heightMap",   MaterialDefaults::WhiteTexture());
        material->floatParams["roughness"] = 0.5f;
        material->floatParams["metallic"] = 0.1f;

        return material;
    }
     */
};

#endif

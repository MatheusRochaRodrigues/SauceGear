#include "ModelLoader.h"
#include "../../Assets/AssetLoader.h"
#include "../../Materials/MaterialLibrary.h"
#include <cassert>
 
static glm::mat4 ConvertMatrix(const aiMatrix4x4& m) {
    glm::mat4 out;
    out[0][0] = m.a1; out[1][0] = m.a2; out[2][0] = m.a3; out[3][0] = m.a4;
    out[0][1] = m.b1; out[1][1] = m.b2; out[2][1] = m.b3; out[3][1] = m.b4;
    out[0][2] = m.c1; out[1][2] = m.c2; out[2][2] = m.c3; out[3][2] = m.c4;
    out[0][3] = m.d1; out[1][3] = m.d2; out[2][3] = m.d3; out[3][3] = m.d4;
    return out;
}

std::shared_ptr<ModelAsset> ModelLoader::Load(const std::string& path) {
    Assimp::Importer importer;
    std::cout << "0" << std::endl;

    //read File and load scene file
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs // |
        //aiProcess_CalcTangentSpace |
        //aiProcess_JoinIdenticalVertices
    );

    if (!scene || !scene->mRootNode)
        return nullptr;

    std::cout << "01" << std::endl;
    auto asset = std::make_shared<ModelAsset>();
    asset->path = path;
    //stem = to get name of the file withouth the last extension -> Example/model.fbx -> return "model"
    asset->name = std::filesystem::path(path).stem().string();
      
    // carrega TODAS as meshes primeiro
    std::cout << "1" << std::endl;

    asset->meshes.resize(scene->mNumMeshes);
    ProcessNodes(
        scene->mRootNode,                       //scene->mMeshes[i]
        scene,
        std::filesystem::path(path).parent_path().string(), // "assets/textures/diffuse.png" -> return "assets/textures"
        asset.get()
    ); 

    std::cout << "20" << std::endl;
    asset->root = ProcessNode(scene->mRootNode);
    std::cout << "21" << std::endl;

    return asset;
}

void ModelLoader::ProcessNodes(
    aiNode* node,
    const aiScene* scene,
    const std::string& dir,
    ModelAsset* modelAsset
){  
    if (node->mNumMeshes > 0) {  

        for (uint32_t i = 0; i < node->mNumMeshes; i++) {
            uint32_t meshIndex = node->mMeshes[i]; 

            if (modelAsset->meshes[meshIndex]) continue; // já processado 

            aiMesh* aiM = scene->mMeshes[meshIndex];
            auto mesh = std::make_shared<MeshAsset>();
            mesh->name = aiM->mName.C_Str();

            std::vector<Vertex>   vertices;
            std::vector<uint32_t> indices;
            std::vector<SubMesh>  submeshes; 
             
            vertices.reserve(aiM->mNumVertices); 
            indices.reserve(aiM->mNumFaces * 3); 
            submeshes.reserve(1); 

            //-------------------------------------------------Extraction----------------------------------------------------------
            // VERTICES 
            for (uint32_t v = 0; v < aiM->mNumVertices; v++) {
                Vertex vert{};
                //Position
                vert.Position = { aiM->mVertices[v].x, aiM->mVertices[v].y, aiM->mVertices[v].z };
                //Normal
                vert.Normal = aiM->HasNormals()
                    ? glm::vec3(aiM->mNormals[v].x, aiM->mNormals[v].y, aiM->mNormals[v].z)
                    : glm::vec3(0, 1, 0);
                //TexCoordenate
                if (aiM->mTextureCoords[0]) {
                    vert.TexCoords = {
                        aiM->mTextureCoords[0][v].x,
                        aiM->mTextureCoords[0][v].y
                    };
                    if (aiM->mTangents) {
                        vert.Tangent =   { aiM->mTangents[v].x,   aiM->mTangents[v].y,    aiM->mTangents[v].z  };
                        vert.Bitangent = { aiM->mBitangents[v].x, aiM->mBitangents[v].y, aiM->mBitangents[v].z };
                    }
                }
                else vert.TexCoords = { 0.0f, 0.0f }; 
                //Bones
                for (int j = 0; j < MAX_BONE_INFLUENCE; ++j) {
                    vert.m_BoneIDs[j] = 0;
                    vert.m_Weights[j] = 0.0f;
                }

                vertices.push_back(vert);
            } 

            // ---------- INDICES ---------- 
            for (uint32_t f = 0; f < aiM->mNumFaces; f++) {
                const aiFace& face = aiM->mFaces[f];
                for (uint32_t j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            } 

            // ---------- SUBMESH (1 material por aiMesh) ----------
            SubMesh sm{};
            sm.indexOffset = 0;
            sm.indexCount = static_cast<uint32_t>(indices.size());
            sm.name = aiM->mName.C_Str();
             
            sm.indexMaterialAsset =
                GetOrCreateMaterialIndex(
                    modelAsset,
                    scene->mMaterials[aiM->mMaterialIndex],
                    dir
                );

            /*sm.materialAsset =
                CreateMaterialAssetFromAssimp(
                    scene->mMaterials[aiM->mMaterialIndex],
                    dir,
                    modelAsset->name
                );*/ 

            submeshes.push_back(sm);
             
            // make reality - setupMesh()
            mesh->vertices  = std::move(vertices);
            mesh->indices   = std::move(indices);
            mesh->submeshes = std::move(submeshes); 
            mesh->UploadToGPU();

            modelAsset->meshes[meshIndex] = mesh;  
        } 
    }

    //traverse the children of this node
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        ProcessNodes(node->mChildren[i], scene, dir, modelAsset);       //modelAsset->meshes.push_back(ProcessNodes(node->mChildren[i], scene, dir, modelAsset));
}  

uint32_t ModelLoader::GetOrCreateMaterialIndex(
    ModelAsset* model,
    aiMaterial* aiMat,
    const std::string& directory
) {
    std::string key = model->name + "::" + aiMat->GetName().C_Str();

    // O(1) average
    auto it = model->materialsLUT.find(key);
    if (it != model->materialsLUT.end()) return it->second;

    // cria novo material
    auto material =
        CreateMaterialAssetFromAssimp(aiMat, directory, model->name);

    uint32_t index = static_cast<uint32_t>(model->materials.size());

    model->materials.push_back(material);
    model->materialsLUT.emplace(key, index);

    return index;
}


 
std::shared_ptr<MaterialAsset>
ModelLoader::CreateMaterialAssetFromAssimp(
    aiMaterial* aiMat,
    const std::string& directory,
    const std::string& modelName
) {
    std::string key = modelName + "::" + aiMat->GetName().C_Str();

    return AssetDatabase::Load<MaterialAsset>(key, [&]() {
        auto asset = std::make_shared<MaterialAsset>();
        asset->name = key;  // name
         
        // base material (shader + layout)
        //  Material base vem do registry
        asset->base = MaterialLibrary::Get("PBR_Default");
        ASSERT(asset->base && "MaterialBase inexistente");
        
        auto TrySetTexture = [&](aiTextureType type, const char* paramName, float fallback = 0, bool isSRGB = false)
        {
            aiString tex;
            if (aiMat->GetTexture(type, 0, &tex) == AI_SUCCESS) 
                asset->defaults[paramName].data = TextureCache::Get().Load(directory + "/" + tex.C_Str(), isSRGB);
            else
                asset->defaults[paramName].data = fallback;

        };

        // Fallback
        aiColor3D color(1, 1, 1);
        if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            asset->defaults["Albedo"].data =
                glm::vec3(color.r, color.g, color.b);
        }   

        // Texturas (se existirem)
        TrySetTexture(aiTextureType_DIFFUSE, "Albedo", 0, true);
        TrySetTexture(aiTextureType_METALNESS, "Metallic", 0.1f);
        TrySetTexture(aiTextureType_DIFFUSE_ROUGHNESS, "Roughness", 0.5f);

        return asset;
    });
} 


std::shared_ptr<HierarchyNode> ModelLoader::ProcessNode(aiNode* node) {
    auto out = std::make_shared<HierarchyNode>();

    out->name = node->mName.C_Str();
    out->localTransform = ConvertMatrix(node->mTransformation);

    for (uint32_t i = 0; i < node->mNumMeshes; i++)
        out->meshIndices.push_back(node->mMeshes[i]);

    for (uint32_t i = 0; i < node->mNumChildren; i++)
        out->children.push_back( ProcessNode(node->mChildren[i]) );

    return out;
}
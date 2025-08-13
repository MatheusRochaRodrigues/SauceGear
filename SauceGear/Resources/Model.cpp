#include "Model.h" 
  
Model::Model(string const& path, bool gamma) : gammaCorrection(gamma)
{
    loadModel(path);
}

Model::Model( )
{ 
}
// draws the model, and thus all its meshes
void Model::Draw()
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw();
}
     
// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void Model::loadModel(string const& path)
{
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        //throw std::runtime_error("Erro ao carregar modelo: " + std::string(importer.GetErrorString()));
        cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
        return;
    }
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));



    //// Carrega todos os materiais da cena uma única vez
    //materials.resize(scene->mNumMaterials);
    //for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
    //    materials[i] = LoadMaterial(scene->mMaterials[i], directory);
    //}


    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
}

//Material* LoadMaterial(aiMaterial* aiMat, const std::string& directory) {
//    Material* material = new Material();
//
//    aiColor3D color(0.f, 0.f, 0.f);
//    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
//        material->albedoColor = glm::vec3(color.r, color.g, color.b);
//    }
//
//    aiString texPath;
//    if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) { 
//
//    } else {
//        material->textures.insert((MaterialDefaults::TextureColor(color.r, color.g, color.b));
//        material->useTexture = false;
//    }
//
//    return material;
//}


// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::processNode(aiNode* node, const aiScene* scene)
{
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }

}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    /*
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN


    // 1. diffuse maps
    vector<Texture> diffuseMaps =   loadMaterialTextures(material, aiTextureType_DIFFUSE,  "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    vector<Texture> specularMaps =  loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    vector<Texture> normalMaps =    loadMaterialTextures(material, aiTextureType_HEIGHT,   "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    vector<Texture> heightMaps =    loadMaterialTextures(material, aiTextureType_AMBIENT,  "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    */

    //Shader* defaultShader = GEngine->renderer->GetDefaultShader(); // você deve definir isso
    Material* UltmMaterial;        //CreateMaterialFrom(material, directory, defaultShader);
    //case him exists
    if (materialCache.find(mesh->mMaterialIndex) != materialCache.end()) {
        UltmMaterial = materialCache[mesh->mMaterialIndex]; 
    }else {
        UltmMaterial = CreateMaterialFrom(material, directory);
        materialCache[mesh->mMaterialIndex] = UltmMaterial;
    }

    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, UltmMaterial);                       //return Mesh(vertices, indices, textures);
} 

 
 
Material* Model::CreateMaterialFrom(aiMaterial* aiMat, const std::string& directory) { 
    auto* material = new Material();                    //auto* material = new Material(defaultShader);

    unsigned int unitTex = 0;

    auto LoadSingleTexture = [&](aiTextureType type, const std::string& uniformName, Texture* fallback) {
        aiString str;
        //check if this mesh has some texture
        if (aiMat->GetTexture(type, 0, &str) == AI_SUCCESS) {
            std::string fullPath = directory + "/" + str.C_Str();

            // Verifica se já foi carregada
            for (const auto& tex : textures_loaded) {
                if (tex.path == str.C_Str()) {
                    material->textures[uniformName] = new Texture(tex); // ou compartilhe ponteiro
                    //material->textures[uniformName] = *tex; // ou compartilhe ponteiro
                    return;
                }
            }

            // Se não, carrega e adiciona
            Texture* tex = new Texture();
            tex->unit = unitTex++;
            tex->LoadFromFile(fullPath.c_str());
            material->textures[uniformName] = tex;

            //salva textura no banco
            tex->path = str.C_Str();
            //std::cout << std::endl << " carregado " << tex->path;
            //std::cout << std::endl << " carregado 2" << material->textures[uniformName]->ID;
            textures_loaded.push_back(*tex); // salva no cache
        }
        else {
            aiColor3D color(0.f, 0.f, 0.f);
            if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
                material->albedoColor = glm::vec3(color.r, color.g, color.b);
            }
            material->textures[uniformName] = MaterialDefaults::TextureColor(color.r, color.g, color.b);
            std::cout << std::endl << " carregado SPECIAL";
            //material->textures[uniformName] = fallback;
        }
    };

    LoadSingleTexture(aiTextureType_DIFFUSE,  "albedoMap",   MaterialDefaults::WhiteTexture());
    //LoadSingleTexture(aiTextureType_SPECULAR, "specularMap", MaterialDefaults::WhiteTexture());
    //LoadSingleTexture(aiTextureType_HEIGHT,   "normalMap",   MaterialDefaults::WhiteTexture());
    //LoadSingleTexture(aiTextureType_AMBIENT,  "heightMap",   MaterialDefaults::WhiteTexture());
    material->floatParams["roughness"] = 0.5f;
    material->floatParams["metallic"]  = 0.1f;

    return material;
}





// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.ID = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
        }
    }
    return textures;
};


unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}



//Model* Model::CreateCustomModel(const std::vector<float>& verts, const std::vector<unsigned>& indices) {
//    Mesh mesh;
//    mesh.vertices = verts;
//    mesh.indices = indices;
//    mesh.SetupMesh();
//    auto* model = new Model();
//    model->AddMesh(mesh);
//    return model;
//}

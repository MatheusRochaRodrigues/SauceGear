#include "AssetLoader.h"
#include "AssetDatabase.h" 
#include "MaterialAsset.h"
#include "../Serializer/MaterialSerializer.h" 
#include "ShaderAsset.h"      
//#include "../Serializer/ShaderSerializer.h" 
#include "../Resources/Loaders/ModelLoader.h"

 
//---Material--------------------- 
template<> std::shared_ptr<MaterialAsset> LoadAsset<MaterialAsset>(const std::string& path) {
    return AssetDatabase::Load<MaterialAsset>(
        path,
        [&] { return MaterialSerializer::Load(path); }
    );
}

 
//---Model------------------------ 
template<> std::shared_ptr<ModelAsset> LoadAsset<ModelAsset>(const std::string& path) {
    return AssetDatabase::Load<ModelAsset>(
        path,
        [&] { return ModelLoader::Load(path); }
    );
}












//template<> std::shared_ptr<ModelAsset> LoadAsset<ModelAsset>(const std::string& path) {
//    return AssetDatabase::Load<ModelAsset>(
//        path,
//        [&] { return ModelLoader::Load(path); }
//    );
//}



/* 
//--------------------------------
//---Shader-----------------------
//--------------------------------
template<> std::shared_ptr<ShaderAsset> LoadAsset<ShaderAsset>(const std::string& path) {
    return AssetDatabase::Load<ShaderAsset>(
        path,
        [&] { return ShaderSerializer::Load(path); }
    );
}
*/

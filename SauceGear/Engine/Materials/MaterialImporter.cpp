//#include"../Assets/MaterialAsset.h"
//#include"../Resources/Loaders/ModelLoader.h"
//
//MaterialAsset* ImportMaterial(aiMaterial* aiMat) {
//    auto asset = new MaterialAsset();
//    asset->base = AssetDatabase::Load<PBRMaterial>("PBR_Default");
//
//    // defaults
//    asset->defaults["Metallic"].data = 0.1f;
//    asset->defaults["Roughness"].data = 0.5f;
//
//    return asset;
//}
//
//
//MaterialAsset CreateMaterialAssetFromAssimp(aiMaterial* aiMat) {
//    MaterialAsset asset;
//
//    asset.base = MaterialRegistry::Get("PBR_Default");
//
//    asset.defaults["Metallic"].data = 0.0f;
//    asset.defaults["Roughness"].data = 0.6f;
//    asset.defaults["Albedo"].data = glm::vec4(1, 1, 1, 1);
//
//    return asset;
//}

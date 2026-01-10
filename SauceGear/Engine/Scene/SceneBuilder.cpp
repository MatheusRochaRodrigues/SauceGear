#include "SceneBuilder.h"
#include "../Resources/Primitives/Primitive.h"
#include "../Materials/MaterialLibrary.h"
 
Entity SceneBuilder::CreateCube() {
    auto& scene = *GEngine->scene;
    Entity e = scene.CreateEntity();

    //auto& mr = scene.AddComponent<MeshRenderer>(e);
    //mr.mesh = std::make_shared<MeshInstance>(PrimitiveMesh::Cube());

    ///*auto po = 
    //    AssetDatabase::Load<MaterialInstance>("PBR_Default", []()
    //        { return std::make_shared<MaterialInstance>( std::make_shared<MaterialAsset>(std::make_shared<PBRMaterial>()) ); });*/

    //auto s = std::make_shared<MaterialAsset>();
    //s->base = MaterialLibrary::Get("PBR_Default"); 
    //mr.materials.push_back(s->Instantiate(s)); 
    
    //MaterialLibrary::Get("PBR_Default")
     
    return e;
}

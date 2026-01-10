#include "MaterialInstance.h"
#include "../Assets/MaterialAsset.h"   // definição completa aqui
 
MaterialInstance::MaterialInstance(const std::shared_ptr<MaterialAsset>& b)
    : asset(b), overrides(asset->defaults) {
}





//MaterialInstance::MaterialInstance(std::shared_ptr<MaterialAsset> a)
//    : asset(std::move(a)), overrides(asset->defaults) {
//}


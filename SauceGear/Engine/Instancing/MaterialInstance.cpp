#include "MaterialInstance.h"
#include "../Assets/MaterialAsset.h"   // definição completa aqui

MaterialInstance::MaterialInstance(std::shared_ptr<MaterialAsset> a)
    : asset(std::move(a)), overrides(asset->defaults) {
}

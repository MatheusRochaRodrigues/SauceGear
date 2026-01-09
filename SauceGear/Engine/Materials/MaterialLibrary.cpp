#include "MaterialLibrary.h"
#include "PBRMaterial.h"

void MaterialLibrary::InitMaterials() {   
    //INCLUSE - CONSTRUCTOR -> pbr->Init(); //cria shader, layout, etc
    MaterialLibrary::Register("PBR_Default", std::make_shared<PBRMaterial>());

}
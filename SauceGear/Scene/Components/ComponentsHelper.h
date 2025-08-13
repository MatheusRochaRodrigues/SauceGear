#pragma once

#include <vector>

#include "Transform.h"
#include "MeshRenderer.h" 
#include "Material.h" 
#include "Velocity.h" 
#include "LightComponent.h" 
#include "CameraComponent.h" 
#include "PostProcessComponent.h"  
#include "GlobalUniformComponent.h"  
//#include "NativeScriptComponent.h"  
#include "HierarchyComponent.h"  


// Aqui você pode criar um alias útil:					
using AllComponentTypes = std::tuple<Transform, Velocity>;



//exemplo
//	Um alias é um nome alternativo para um tipo, criado usando using ou typedef.
//		using Vec = std::vector<int>;
//		Vec v = {1, 2, 3};  // em vez de std::vector<int>
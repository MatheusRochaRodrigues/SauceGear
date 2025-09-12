#include "../Scene/Entity.h"
#include <iostream>

#pragma once
struct NameComponent { 
    std::string name;
};

struct HierarchyComponent {
    Entity parent = INVALID_ENTITY;
    Entity firstChild = INVALID_ENTITY;
    Entity nextSibling = INVALID_ENTITY;
};

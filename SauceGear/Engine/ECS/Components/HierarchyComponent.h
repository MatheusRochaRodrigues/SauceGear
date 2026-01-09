#pragma once
#include "../../Scene/Entity.h"
#include <iostream>

struct NameComponent { 
    std::string name;
};

struct HierarchyComponent {
    Entity parent = INVALID_ENTITY;
    Entity firstChild = INVALID_ENTITY;
    Entity nextSibling = INVALID_ENTITY;
};

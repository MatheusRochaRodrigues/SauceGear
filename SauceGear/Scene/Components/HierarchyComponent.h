#include "../Entity.h"
#include <iostream>

#pragma once
struct NameComponent {
    //NameComponent(string name) { this->name = name; };
    std::string name;
};

struct HierarchyComponent {
    Entity parent = INVALID_ENTITY;
    Entity firstChild = INVALID_ENTITY;
    Entity nextSibling = INVALID_ENTITY;
};

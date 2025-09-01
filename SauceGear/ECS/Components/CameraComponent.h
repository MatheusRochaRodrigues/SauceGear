#pragma once

#include "../../Core/Camera.h"

struct CameraComponent {

    Camera* camera = new Camera(); //Camera* camera = nullptr;
    bool isMain = false;  // Define se essa câmera é a principal

    CameraComponent(bool main = false) : isMain(main) { } 

};

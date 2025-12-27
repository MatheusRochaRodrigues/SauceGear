#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp> 
#include <iostream>  

#define MAX_LIGHTS 16

enum class ShadowLOD {
    HIGH,
    MEDIUM,
    LOW,
    NONE
};

enum class LightType { Directional, Point, Spot };

struct LightComponent {                             // base alignment       // aligned offset
    LightType type = LightType::Point;              //4                     //0 
    glm::vec3 position = glm::vec3(0.0f);           //16                    //16
    glm::vec3 color = glm::vec3(1.0f);              //16                    //32 
    float intensity = 1.0f;                         //4                     //48    
    float range = 25.0f;                            //4                     //52
    float angle = 45.0f;     // Para Spot           //4                     //56 
    bool castShadow = true;                         //4                     //60 
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);   //16                    //64
                                                    //16                    //80
                                                    //16                    //96
                                                    //16                    //112 
    GLuint depthMap = 0;                            //4                     //128
                                                    //end                   //132
                                                    //multilpy of vec4(16)  //144 bytes

    void SetTypeLight(LightType type) {
        this->type = type;  
        this->range = (type == LightType::Directional) ? 7.5f : 25.0f;
        this->position = (type == LightType::Directional) ? glm::vec3(-2.0f, 4.0f, -1.0f) : glm::vec3(0.0f);
    }; 


    REFLECT_CLASS(LightComponent) {
        REFLECT_HEADER("LightComponent");
        REFLECT_FIELD(color);
        REFLECT_FIELD(intensity);
    }
};

//glm::mat4 shadowTransforms[6] = {};             //16                    //80
////16                    //96
////16                    //112
////16                    //128
////16                    //144
////16                    //160 

// Shadow setup

//float cutoff = 12.5f;  // para spotlight        //4                     //
//float outerCutoff = 1;  // para spotlight       //4                     //
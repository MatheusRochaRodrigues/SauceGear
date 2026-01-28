#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp> 
#include <iostream>   
#include "../Reflection/Macros.h"
#include "../../Data/Color.h"
  
enum class ShadowLOD { HIGH, MEDIUM, LOW, NONE }; 
enum class LightType { Directional, Point, Spot };

#define MAX_LIGHTS 16

struct LightComponent {                              
    LightType   type = LightType::Point;               
    glm::vec3   position = glm::vec3(0.0f);            
    glm::vec3   color = glm::vec3(1.0f);            //glm::vec3   color = glm::vec3(1.0f);                 
    float       intensity = 1.0f;                           
    float       range = 25.0f;                            
    float       angle = 45.0f;     // Para Spot            
    bool        castShadow = true;                         
    glm::mat4   lightSpaceMatrix = glm::mat4(1.0f);                                 
    GLuint      depthMap = 0;       

    float       intensityBillboard = 25.0f;

    REFLECT_CLASS(LightComponent) {
        REFLECT_HEADER("LightComponent");
        
        REFLECT_FIELD_COLOR(color)
        //REFLECT_FIELD(color)
            //.UI(EditorWidget::Color);
        
        REFLECT_FIELD(intensity);

        REFLECT_FLOAT_SLIDER(intensityBillboard, 0.0f, 100.0f);   // cromaticidade

        REFLECT_ADD_COMPONENT();
    }

    void SetTypeLight(LightType type) {
        this->type = type;  
        this->range = (type == LightType::Directional) ? 7.5f : 25.0f;
        this->position = (type == LightType::Directional) ? glm::vec3(-2.0f, 4.0f, -1.0f) : glm::vec3(0.0f);
    };  

private:

};
 













// Shadow setup 
//float cutoff = 12.5f;  // para spotlight        //4                     //
//float outerCutoff = 1;  // para spotlight       //4                     //
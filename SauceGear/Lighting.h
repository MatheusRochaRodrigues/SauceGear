#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H
  
#include "Shader.h"  
#include <algorithm> 

#define MAX_LIGHTS 4

struct Light { 
    enum class TypeLight {
        Directional,
        Point,
        Spot
    };

    glm::vec3 position;
    glm::vec3 color;
    glm::mat4 model;
    TypeLight type;

    //Pos Processing
    GLuint depthMap;
    glm::mat4 lightSpaceMatrices;
};

class Lighting {
public:
    Lighting(Shader& Directional, const unsigned int SHADOW_WIDTH = 1024, const unsigned int SHADOW_HEIGHT = 1024);

    //LightManager(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& colors);
    void InstanceDirectionalLight   (glm::vec3 position, glm::mat4 model, glm::vec3 color);  
    void InstancePointLight         (glm::vec3 position, glm::mat4 model, glm::vec3 color);
    void InstanceSpotLight          (glm::vec3 position, glm::mat4 model, glm::vec3 color);

    //void UpdateLights(const Shader& shader, float time);
    void UpdateLights(glm::vec3 positionPlayer);
    std::vector<Light*> getLightsNearest(glm::vec3 positionPlayer);

    std::vector<Light> allLights;
    Shader* DirectionalShader;
private:
    Shader* PointShader;

    unsigned int SHADOW_WIDTH;
    unsigned int SHADOW_HEIGHT;

    unsigned int depthMapFBO;   //FBO_depthLightMap

    //unsigned int depthMap; 
    //std::vector<GLuint> depthMaps;

    float near_plane = 1.0f, far_plane = 7.5f;


    //std::vector<glm::vec3> positions;
    //std::vector<glm::vec3> colors;
};

#endif // LIGHT_MANAGER_H
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
    
    //dir
    glm::mat4 lightSpaceMatrices;

    //point
    glm::mat4 shadowTransforms[6];
};

class Lighting {
public:
    //Lighting(Shader& Directional, Shader& Point, const unsigned int SHADOW_WIDTH = 512, const unsigned int SHADOW_HEIGHT = 512);
    Lighting(Shader& Directional, Shader& Point, const unsigned int SHADOW_WIDTH = 1024, const unsigned int SHADOW_HEIGHT = 1024);

    //LightManager(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& colors);
    void InstanceDirectionalLight   (glm::vec3 position, glm::mat4 model, glm::vec3 color);  
    void InstancePointLight         (glm::vec3 position, glm::mat4 model, glm::vec3 color);
    void InstanceSpotLight          (glm::vec3 position, glm::mat4 model, glm::vec3 color);

    //void UpdateLights(const Shader& shader, float time);
    void UpdateLights(glm::vec3 positionPlayer);
    void updtDirectional(Light& light);
    void updtPoint(Light& light);

    std::vector<Light*> getLightsNearest(glm::vec3 positionPlayer);

    std::vector<Light> allLights;
    Shader* DirectionalShader;
    Shader* PointShader;

    //float near_plane = 1.0f, far_plane = 7.5f;
    float near_plane = 1.0f;
    float far_plane = 25.0f;
private:

    unsigned int SHADOW_WIDTH;
    unsigned int SHADOW_HEIGHT;

    unsigned int depthMapFBO;   //FBO_depthLightMap

    //unsigned int depthMap; 
    //std::vector<GLuint> depthMaps;



    //std::vector<glm::vec3> positions;
    //std::vector<glm::vec3> colors;
};

#endif // LIGHT_MANAGER_H
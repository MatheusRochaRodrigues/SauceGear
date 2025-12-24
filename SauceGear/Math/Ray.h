#pragma once 
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_inverse.hpp> 

//after implement CoreModule that include it

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction; // sempre normalizado

    Ray() = default;

    Ray(const glm::vec3& o, const glm::vec3& d)
        : origin(o), direction(glm::normalize(d)) {
    }

    inline glm::vec3 at(float t) const {
        return origin + direction * t;
    }
};

 

class RayFactory {
public: 

    //Isso pega a posição do mouse, transforma de NDC para world space, e retorna um raio.
    static Ray ScreenPosToWorldRay(
        float mouseX, float mouseY,
        float screenWidth, float screenHeight,
        const glm::mat4& view, const glm::mat4& projection,
        const glm::vec3& cameraPos)
    {
        // NDC
        float x = (2.0f * mouseX) / screenWidth - 1.0f;
        float y = 1.0f - (2.0f * mouseY) / screenHeight;

        glm::vec4 nearPointNDC(x, y, -1.0f, 1.0f);
        glm::vec4 farPointNDC(x, y, 1.0f, 1.0f);

        glm::mat4 invVP = glm::inverse(projection * view);

        glm::vec4 nearWorld = invVP * nearPointNDC;     nearWorld /= nearWorld.w;
        glm::vec4 farWorld = invVP * farPointNDC;      farWorld /= farWorld.w;
         
        return Ray(cameraPos, glm::normalize(glm::vec3(farWorld - nearWorld)));
    }

};


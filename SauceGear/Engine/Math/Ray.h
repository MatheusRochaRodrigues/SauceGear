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
    // Recomended - The Better get Ray of the screen
    // By default, the standard GLFW coordinate system is assumed, 
    // which defines the upper left corner (0,0) of the window as the cursor point.
    static Ray ScreenPosToWorldRay(
        float mouseX, float mouseY,
        float screenWidth, float screenHeight,
        const glm::mat4& view, const glm::mat4& projection )
    {
        // NDC
        float x = (2.0f * mouseX) / screenWidth - 1.0f;
        //se usa 1.0f - f(x) devido a natureza de que a janela(GLFW)/imgui usa como cordenada inicial de canto superior esquerdo(0,0)-
        // e o OpenGL espera que em NDC o topo de Y seja 1(maior) e nao o menor
        float y = 1.0f - (2.0f * mouseY) / screenHeight;

        glm::vec4 nearPointNDC(x, y, -1.0f, 1.0f);
        glm::vec4 farPointNDC(x, y, 1.0f, 1.0f);

        glm::mat4 invVP = glm::inverse(projection * view);

        glm::vec4 nearWorld = invVP * nearPointNDC;     nearWorld /= nearWorld.w;
        glm::vec4 farWorld = invVP * farPointNDC;      farWorld /= farWorld.w;
          
        return Ray(nearWorld, glm::normalize(glm::vec3(farWorld - nearWorld)));
    }

    static Ray ScreenToWorldRay_NotFlipY(
        float mouseX, float mouseY,
        float screenWidth, float screenHeight,
        const glm::mat4& view, const glm::mat4& projection)
    {
        // NDC
        float x = (2.0f * mouseX) / screenWidth - 1.0f; 
        float y = (2.0f * mouseY) / screenHeight - 1.0f; // NÃO inverte

        glm::vec4 nearPointNDC(x, y, -1.0f, 1.0f);
        glm::vec4 farPointNDC(x, y, 1.0f, 1.0f);

        glm::mat4 invVP = glm::inverse(projection * view);

        glm::vec4 nearWorld = invVP * nearPointNDC;     nearWorld /= nearWorld.w;
        glm::vec4 farWorld = invVP * farPointNDC;      farWorld /= farWorld.w;

        return Ray(nearWorld, glm::normalize(glm::vec3(farWorld - nearWorld)));
    }


    //LEGACY - NAO USE 
    //static Ray ScreenPosToWorldRay(
    //    float mouseX, float mouseY,
    //    float screenWidth, float screenHeight,
    //    const glm::mat4& view, const glm::mat4& projection,
    //    const glm::vec3& cameraPos)
    //{
    //    // NDC
    //    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    //    float y = 1.0f - (2.0f * mouseY) / screenHeight; 
    //    glm::vec4 farPointNDC(x, y, 1.0f, 1.0f); 

    //    glm::mat4 invVP = glm::inverse(projection * view); 
    //    glm::vec4 farWorld = invVP * farPointNDC;      farWorld /= farWorld.w;
    //     
    //    return Ray(cameraPos, glm::normalize(glm::vec3(farWorld) - cameraPos)); 
    //}


    

};




/*
//Estranho  Alternativa a ser analisa e duvidosa
static Ray ScreenToWorldRay_Yflip(
    float mouseX, float mouseY,
    float viewportWidth, float viewportHeight,
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& camPos
) {
    // 1. Mouse -> NDC [-1, 1]
    float x = (2.0f * mouseX) / viewportWidth - 1.0f;
    float y = (2.0f * mouseY) / viewportHeight - 1.0f;
    y = -y; // ImGui Y-flip

    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

    // 2. Clip -> View
    glm::vec4 rayView = glm::inverse(projection) * rayClip;
    rayView.z = -1.0f;
    rayView.w = 0.0f;

    // 3. View -> World
    glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayView));

    Ray ray;
    ray.origin = camPos;
    ray.direction = rayWorld;
    return ray;
}
*/








/*
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

    //alternativa
    //return Ray(cameraPos, glm::normalize(glm::vec3(farWorld) - cameraPos));

    return Ray(cameraPos, glm::normalize(glm::vec3(farWorld - nearWorld)));
}
*/
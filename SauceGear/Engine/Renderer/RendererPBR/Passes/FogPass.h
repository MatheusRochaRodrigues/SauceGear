#pragma once
#include <glm/glm.hpp> 

class Shader;
class Framebuffer;

class FogPass {
public:
    FogPass(Shader* shader) : shader(shader) {};
    void Execute(Framebuffer* target, Framebuffer* gbuffer, glm::vec3 camPos);

private:
    Shader* shader;
};

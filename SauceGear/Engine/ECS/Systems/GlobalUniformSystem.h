#pragma once
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>

class GlobalUniformSystem : public System {
public:
    GlobalUniformSystem() {
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Fixa na binding point 0 (por exemplo)
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
        //glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, 2 * sizeof(glm::mat4));                             
    }

    void Update(float time) override { 
        //GlobalUniforms data;
        //data.time = time;
        //data.cameraPosition = GEngine->mainCamera->Position;
        
        //bind
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);

        //projection
        glm::mat4 projection = GEngine->mainCamera->GetProjectionMatrix();
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        //view
        glm::mat4 view = GEngine->mainCamera->GetViewMatrix();
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

        //unbind
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void Bind(GLuint bindingPoint = 0) {
        //glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, ubo, 0, sizeof(GlobalUniforms));
    }

    GLuint GetUBO() const { return ubo; }

private:
    GLuint ubo = 0;
};



//define ubo for shader program example 
//unsigned int uniformBlockIndexYellow = glGetUniformBlockIndex(material->shader->ID, "Matrices");
//glUniformBlockBinding(material->shader->ID, uniformBlockIndexYellow, 0);
#pragma once  
#include <algorithm>
#include <glm/glm.hpp> 
#include "OperationsSDF.h" 

class SignedDistanceField {   
public:
    virtual ~SignedDistanceField() = default;
    virtual float sdfDistance(const glm::vec3& pos) const = 0;

    inline glm::vec3 normal(const glm::vec3& pos) const
    {
        const static float delta = 0.001f;
        const static glm::vec3 xyy(delta, -delta, -delta);
        const static glm::vec3 yyx(-delta, -delta, delta);
        const static glm::vec3 yxy(-delta, delta, -delta);
        const static glm::vec3 xxx(delta, delta, delta);
        return glm::normalize(xyy * sdfDistance(pos + xyy) + yyx * sdfDistance(pos + yyx) + yxy * sdfDistance(pos + yxy) +
            xxx * sdfDistance(pos + xxx));
    }  
protected:
    static void _bind_methods()
    {
        // Binding methods for Godot         
    }
}; 
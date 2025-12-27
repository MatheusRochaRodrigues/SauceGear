#pragma once 
#include <glm/glm.hpp>
#include <vector>

enum class DebugPointType : uint8_t {
    Square = 0,
    Circle = 1
};

struct DebugLine {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 color;
};

struct DebugPoint {
    glm::vec3 pos;
    glm::vec3 color;
    float size;
    DebugPointType type;
};

class DebugDraw {
public:
    // acesso global
    static DebugDraw& Get() {
        static DebugDraw instance;
        return instance;
    }

    // ========= API pública =========
    void Line(const glm::vec3& a,
        const glm::vec3& b,
        const glm::vec3& color = { 1,1,1 },
        bool persistent = false);

    void Point(const glm::vec3& p,
        const glm::vec3& color = { 1,1,1 },
        float size = 6.0f,
        DebugPointType type = DebugPointType::Square,
        bool persistent = false);

    // ========= consumo pelo renderer =========
    const std::vector<DebugLine>& GetTempLines() const { return tempLines; }
    const std::vector<DebugLine>& GetPersistentLines() const { return persistentLines; }

    const std::vector<DebugPoint>& GetTempPoints() const { return tempPoints; }
    const std::vector<DebugPoint>& GetPersistentPoints() const { return persistentPoints; }
     
    void ClearTemp() {
        tempLines.clear();
        tempPoints.clear();
    }

private:
    DebugDraw() = default;

    std::vector<DebugLine>  tempLines;
    std::vector<DebugLine>  persistentLines;
    std::vector<DebugPoint> tempPoints;
    std::vector<DebugPoint> persistentPoints;
};

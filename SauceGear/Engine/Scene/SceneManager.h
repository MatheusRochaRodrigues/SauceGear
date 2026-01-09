#pragma once
#include "SceneECS.h"

class SceneManager {
public:

    void RegisterScene(const std::string& name, std::unique_ptr<SceneECS> scene) {
        scenes[name] = std::move(scene);
    }

    void SetActiveScene(const std::string& name) {
        currentScene = name;
    }

    SceneECS* GetActiveScene() {
        return scenes[currentScene].get();
    }

    void ReloadActiveScene() {
        if (!currentScene.empty()) {
            auto old = std::move(scenes[currentScene]);
            scenes[currentScene] = std::make_unique<SceneECS>();
            //*scenes[currentScene] = *old;
        }
    }


private:
    std::unordered_map<std::string, std::unique_ptr<SceneECS>> scenes;
    std::string currentScene;
};

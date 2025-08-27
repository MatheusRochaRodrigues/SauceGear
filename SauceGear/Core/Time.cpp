#include "Time.h"

float Time::s_DeltaTime = 0.0f;
float Time::s_ElapsedTime = 0.0f;
float Time::s_LastFrameTime = 0.0f;

//FPS - Counter
float Time::timer = 0.0f;
int Time::frames = 0;

unsigned int Time::frameCount = 0;

float Time::fps = 0.0f;
float Time::fpsMin = 9999.0f;
float Time::fpsMax = 0.0f;

void Time::Init() {
    s_LastFrameTime = (float)glfwGetTime();
}

void Time::Update() {
    float currentTime = (float)glfwGetTime();
    s_DeltaTime = currentTime - s_LastFrameTime;
    s_LastFrameTime = currentTime;

    s_ElapsedTime += s_DeltaTime;




    //FPS - Counter
    float fpsInstant = (s_DeltaTime > 0.0f) ? (1.0f / s_DeltaTime) : 0.0f;

    frames++;
    timer += s_DeltaTime;

    // Atualiza min/max
    if (fpsInstant < fpsMin) fpsMin = fpsInstant;
    if (fpsInstant > fpsMax) fpsMax = fpsInstant;

    if (timer >= 1.0f) {
        fps = frames / timer; // média do último segundo
        frames = 0;
        timer = 0.0f;
        //fpsMin = 9999.0f;
        //fpsMax = 0.0f;
    }

    
    //first form
    /*
    //FPS - Counter
    frames++;
    timer += s_DeltaTime;           //Time::GetDeltaTime();
    if (timer >= 1.0f) {
        //std::cout << "FPS: " << frames << std::endl;
        frames = 0;
        timer = 0.0f;
    } 
    */

    frameCount++;
}

float Time::GetDeltaTime() {
    return s_DeltaTime;
}

float Time::GetTime() {
    return s_ElapsedTime;
}


float Time::GetFPS() {
    //return frames;
    return fps;
}

unsigned int Time::GetFrameCount() {
    return frameCount;
}



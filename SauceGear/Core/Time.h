#ifndef TIME_H
#define TIME_H 

#include "../Platform/Window.h"
class Time {
public:
    static void Init();              // Chamada uma vez no início
    static void Update();           // Chamada todo frame

    static float GetDeltaTime();    // Tempo entre frames
    static float GetTime();         // Tempo total desde o início

    static float GetFPS();         // Tempo total desde o início

    unsigned int GetFrameCount();
private:
    static float s_DeltaTime;
    static float s_ElapsedTime;
    static float s_LastFrameTime;
     
    static float timer;
    static int frames;

    static unsigned int frameCount;
};

#endif

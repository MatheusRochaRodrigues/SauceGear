#include "RenderDebugSettings.h"
#include "../ECS/Reflection/Meta.h"

// 1 option
/*
BEGIN_ENUM_INFO(RenderViewMode)
ENUM_VALUE(RenderViewMode, FinalLighting)
ENUM_VALUE(RenderViewMode, Albedo)
ENUM_VALUE(RenderViewMode, Normal)
ENUM_VALUE(RenderViewMode, Position)
ENUM_VALUE(RenderViewMode, MRA)
ENUM_VALUE(RenderViewMode, Depth)
END_ENUM_INFO()
*/

 
// 2 option
EnumInfo RenderViewMode_EnumInfo{
    "RenderViewMode",
    {
        { "FinalLighting", (int)RenderViewMode::FinalLighting },
        { "Albedo",        (int)RenderViewMode::Albedo },
        { "Normal",        (int)RenderViewMode::Normal },
        { "Position",      (int)RenderViewMode::Position },
        { "MRA",           (int)RenderViewMode::MRA },
        { "Depth",         (int)RenderViewMode::Depth },
    }
};

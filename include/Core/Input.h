#pragma once
#include "Core/Config.h"
#include <glm/glm.hpp>
#include "KeyCodes.h"
#include "MouseCodes.h"

namespace UE {

    class UE_API Input{
    public:
        static bool IsKeyPressed(KeyCode key);
    
        static bool IsMouseButtonPressed(MouseCode button);
        static glm::vec2 GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();
    };
}
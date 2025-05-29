#pragma once
#include "Core/Config.h"
#include <glm/glm.hpp>
#include "KeyCodes.h"
#include "MouseCodes.h"

namespace UE {

    class  Input{
    public:
        static bool IsKeyPressed(KeyCode key);
    
        static bool IsMouseButtonPressed(MouseCode button);
        static glm::vec2 GetMousePosition();
        static void HideCursor(bool hide);
        static void SetCursorPos(float width, float height);
        static glm::vec2 GetMouseDelta();
        static float GetMouseX();
        static float GetMouseY();
    };
}
#pragma once
#include "Core/Config.h"
#include <glm/glm.hpp>
#include "KeyCodes.h"
#include "MouseCodes.h"

namespace UE {

    class  Input{
    public:
        static bool IsKeyPressed(KeyCode key);
        static bool IsKeyJustPressed(KeyCode key);
        static bool IsMouseButtonPressed(MouseCode button);
        static glm::vec2 GetMousePosition();
        static glm::vec2 GetMouseDelta();
        static float GetMouseX();
        static float GetMouseY();
        static void HideCursor(bool hide);
        static void SetCursorPos(const glm::vec2& pos);
        static void SetCursorPos(float width, float height);
    private:
        static bool s_Keys[512];
        static bool s_KeysLastFrame[512];

        static bool s_MouseButtons[32];

        static glm::vec2 s_MousePos;
        static glm::vec2 s_LastMousePos;
        static glm::vec2 s_MouseDelta;

        // platform hooks
        static std::function<void(bool)> s_SetCursorVisible;
        static std::function<void(float, float)> s_SetCursorPos;

        static void Update(); // called once per frame

        friend class Window; // backend writes state
        friend class Application; // frame update

    };
}
#pragma once

#include "Config.h"

namespace UE {

    class Window{
    public:
        Window() = default;
        ~Window();

        void Init(const int width, const int height, const char* title);
        int GetWidth();
        int GetHeight();    
        int GetFPS();

        bool ShouldClose();
    private:
        int m_Width, m_Height, m_FPS;
    };
}
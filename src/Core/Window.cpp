#include "uepch.h"
#include "Core/Window.h"
#include <raylib.h>
namespace UE {

    Window::~Window(){
        CloseWindow();
    }

    void Window::Init(const int width, const int height,const char* title){
        m_Width = width;
        m_Height = height;

        InitWindow(m_Width, m_Height, title);
        
        m_FPS = 60;
        SetTargetFPS(m_FPS);
    }

    int Window::GetWidth(){ return m_Width;}
    int Window::GetHeight() { return m_Height;}
    int Window::GetFPS() { return m_FPS;}

    bool Window::ShouldClose(){
        return WindowShouldClose();
    }
}
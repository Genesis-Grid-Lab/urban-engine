#pragma once

#include "Core/Config.h"
#include "Events/Event.h"
#include "Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace UE {    

    struct WindowProps{

        std::string m_Title;
        uint32_t m_Width;
        uint32_t m_Height;
    
        WindowProps(const std::string& title = "Urban engine", 
                              uint32_t width = 1600,
                            uint32_t height = 900)
            : m_Title(title), m_Width(width), m_Height(height){
    
            }
    };
    
    class  Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;
    
        Window(const WindowProps& props);
        ~Window();
    
        void OnUpdate();
    
        uint32_t GetWidth() const { return m_Data.Width;}
        uint32_t GetHeight() const { return m_Data.Height;}
    
        //Window attributes
        void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback;}
        void SetVSync(bool enabled);
        bool IsVSync() const;
    
        void* GetNativeWindow() const { return m_Window;}
    
    private:
        void Init(const WindowProps& props);
        void Shutdown();
    
    private:
        GLFWwindow* m_Window;
        Scope<GraphicsContext> m_Context;
    
        struct WindowData{
            std::string Title;
            uint32_t Width, Height;
            bool VSync;
    
            EventCallbackFn EventCallback;
        };
    
        WindowData m_Data;
    };
}


#pragma once

#include "Core/Config.h"
#include "UrbanEngine.h"

#include <Timestep.h>
#include "ImGuiLayer.h"

int main(int argc, char** argv);

namespace UE {

    struct ApplicationCommandLineArgs{
        int Count = 0;
        char** Args = nullptr;
    
        const char* operator[](int index) const{
            UE_CORE_ASSERT(index < Count);
            return Args[index];
        }
    };
    
    enum class LayerActionType{
        Push,
        Pop
    };
    
    struct LayerAction {
        LayerActionType Type;
        Layer* LayerPtr;
    };
    
    
    class UE_API Application{
    public:
        Application(const std::string& name = "Application Name", ApplicationCommandLineArgs args = ApplicationCommandLineArgs());
        virtual ~Application();
    
        void OnEvent(Event& e);
    
        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
        void PopLayer(Layer* layer);
        ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer;}
        
        void Close();
        
        Window& GetWindow() { return *m_Window;}
        static Application& Get() { return *s_Instance;}
    
        ApplicationCommandLineArgs GetCommandLineArgs() const { return m_CommandLineArgs;}
    
        void QueueLayerAction(LayerActionType type, Layer* layer) {
            m_LayerActionQueue.push({ type, layer });
        }
    private:
        void Run();
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);
    private:
        ApplicationCommandLineArgs m_CommandLineArgs;
        Scope<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        float m_LastFrameTime = 0.0f;
        std::queue<LayerAction> m_LayerActionQueue;
    private:
        static Application* s_Instance;
        friend int ::main(int argc, char** argv);
    };

    // To be defined in CLIENT
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
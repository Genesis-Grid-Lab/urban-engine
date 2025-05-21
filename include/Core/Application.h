#pragma once

#include "Config.h"
#include "Layer.h"
#include "LayerStack.h"
#include "ImGuiLayer.h"
#include "Window.h"
#include "Core/UE_Assert.h"

int main(int argc, char** argv);
namespace UE {

    enum class LayerActionType{
        Push,
        Pop
    };

    struct LayerAction{
        LayerActionType Type;
        Layer* LayerPtr;
    };

    class Application {
    public:
        Application(const std::string& name = "Untitle", int width = 0, int height = 0);
        virtual ~Application();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* layer);

        ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer;}

        void Close();

        static Application& Get() { return *s_Instance;}

        void QueueLayerAction(LayerActionType type, Layer* layer) {
            m_LayerActionQueue.push({ type, layer });
        }

    private:
        void Run();
    private:
        Scope<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        std::queue<LayerAction> m_LayerActionQueue;
    private:
        static Application* s_Instance;
        friend int ::main(int argc, char** argv);
    };

    Application* CreateApplication();
}
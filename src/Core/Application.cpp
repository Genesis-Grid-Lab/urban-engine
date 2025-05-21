#include "uepch.h"
#include "Core/Application.h"

namespace UE {

    Application* Application::s_Instance = nullptr;

    Application::Application(const std::string& name, int width, int height){
        UE_CORE_ASSERT(!s_Instance, "Application already exists!");

        s_Instance = this;

        m_Window = CreateScope<Window>();

        m_Window->Init(width, height, name.c_str());

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
    }

    Application::~Application(){

    }

    void Application::PushLayer(Layer* layer)
    {    
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    void Application::PopLayer(Layer* layer) {
        //layer on dettach
        m_LayerStack.PopLayer(layer);
    }

    void Application::PopOverlay(Layer* layer) {
        m_LayerStack.PopOverlay(layer);
    }

    void Application::Close(){
        m_Running = false;
    }


    void Application::Run(){

        while(m_Running && !m_Window->ShouldClose()){
            // float time = (float)glfwGetTime();
            Timestep timestep;
             
            BeginDrawing();
            if(!m_Minimized){
                for(Layer* layer : m_LayerStack)
                    layer->OnUpdate(timestep);
                 
                m_ImGuiLayer->Begin();

                    for(Layer* layer : m_LayerStack)
                        layer->OnImGuiRender();

                m_ImGuiLayer->End();           
            }     
            EndDrawing();       

            while (!m_LayerActionQueue.empty()) {
                LayerAction action = m_LayerActionQueue.front();
                m_LayerActionQueue.pop();
            
                if (action.Type == LayerActionType::Push) {
                    PushLayer(action.LayerPtr);
                } else if (action.Type == LayerActionType::Pop) {
                    PopLayer(action.LayerPtr);
                    delete action.LayerPtr;
                }
            }
        }
    }
}
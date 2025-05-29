#include "uepch.h"
#include "Application.h"
#include <Renderer/Renderer.h>
#include <GLFW/glfw3.h>

namespace UE {

    Application* Application::s_Instance = nullptr;

    Application::Application(const std::string& name, const glm::vec2& size, ApplicationCommandLineArgs args)
        :m_CommandLineArgs(args){

            UE_PROFILE_FUNCTION();
            UE_CORE_ASSERT(!s_Instance, "Application already exists!");

            s_Instance = this;
            m_Window = CreateScope<Window>(WindowProps(name, size.x, size.y));
            m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

            UE::Renderer::Init();

            m_ScreenShader = Shader::Create("Data/Shaders/Screen.glsl");

    #if UE_DEBUG
            m_ImGuiLayer = new ImGuiLayer();
            PushOverlay(m_ImGuiLayer);
    #endif

        }

    Application::~Application(){
        UE_PROFILE_FUNCTION();
        UE::Renderer::Shutdown();
    }

    void Application::PushLayer(Layer* layer)
    {    
        UE_PROFILE_FUNCTION();
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer)
    {
        UE_PROFILE_FUNCTION();
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    void Application::PopLayer(Layer* layer) {
        UE_PROFILE_FUNCTION();
        m_LayerStack.PopLayer(layer);
    }

    void Application::Close(){
        m_Running = false;
    }

    void Application::OnEvent(Event& e){
        UE_PROFILE_FUNCTION();
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.Handled) 
                break;
            (*it)->OnEvent(e);
        }
    }

    void Application::Run(){

        UE_PROFILE_FUNCTION();
        while(m_Running){
            UE_PROFILE_SCOPE("RunLopp");
            float time = (float)glfwGetTime();
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;
                
            if(!m_Minimized){
                {
                    UE_PROFILE_SCOPE("LayerStack OnUpdate");
                    for(Layer* layer : m_LayerStack)
                        layer->OnUpdate(timestep);
                }

    #if UE_DEBUG                
                m_ImGuiLayer->Begin();
                {
                    UE_PROFILE_SCOPE("LayerStack OnImGuiRender");
                    for(Layer* layer : m_LayerStack)
                        layer->OnImGuiRender();
                }

                m_ImGuiLayer->End();
    #endif            
            }

            m_Window->OnUpdate();

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

    bool Application::OnWindowClose(WindowCloseEvent& e){

        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e){

        UE_PROFILE_FUNCTION();

        if(e.GetWidth() == 0 || e.GetHeight() == 0){
            m_Minimized = true;
            return false;
        }

        m_Minimized = false;

        UE::Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

        return false;
    }
}
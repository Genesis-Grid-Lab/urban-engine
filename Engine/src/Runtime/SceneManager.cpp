#include "Runtime/SceneManager.h"

namespace UE {

    SceneManager::SceneManager(uint32_t width, uint32_t height): m_ViewportWidth(width), m_ViewportHeight(height){}

    void SceneManager::LoadScene(Ref<RuntimeScene> scene){
        if(m_ActiveScene)
            m_ActiveScene->OnRuntimeStop();

        m_ActiveScene = scene;

        if(m_ActiveScene)            
            m_ActiveScene->OnRuntimeStart();        
    }

    void SceneManager::CreateEmptyScene(){
        LoadScene(CreateRef<RuntimeScene>(m_ViewportWidth, m_ViewportHeight));
    }

    void SceneManager::Update(Timestep ts){

        if(m_ActiveScene)
            m_ActiveScene->OnUpdate(ts);
    }

    void SceneManager::Render(){
        if(m_ActiveScene)
            m_ActiveScene->Draw();
    }

    void SceneManager::Resize(uint32_t width, uint32_t height){
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        if(m_ActiveScene)
            m_ActiveScene->OnViewportResize(width, height);
    }

    void SceneManager::StopActiveScene(){
        if(m_ActiveScene)
            m_ActiveScene->OnRuntimeStop();
    }
}
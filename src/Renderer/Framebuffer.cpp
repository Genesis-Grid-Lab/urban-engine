#include "uepch.h"
#include "Renderer/Framebuffer.h"
#include <raylib.h>

namespace UE {

    Framebuffer::Framebuffer(int width, int height) : m_Width(width), m_Height(height){

        m_ViewTexture = LoadRenderTexture(m_Width, m_Height);
    }

    Framebuffer::~Framebuffer(){
        UnloadRenderTexture(m_ViewTexture);
    }

    void Framebuffer::Resize(int width, int height){
        if(IsWindowResized()){
            UnloadRenderTexture(m_ViewTexture);
            m_ViewTexture = LoadRenderTexture(width, height);
        }
    }

    void Framebuffer::Bind(){
        BeginTextureMode(m_ViewTexture);
    }

    void Framebuffer::Unbind(){
        EndTextureMode();
    }
}
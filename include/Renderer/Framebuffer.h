#pragma once

#include "Core/Config.h"

namespace UE {

    class Framebuffer{
    public:
        Framebuffer(int width, int height);
        ~Framebuffer();

        void Resize(int width, int height);
        void Unbind();
        void Bind();

        RenderTexture &GetTexture() { return m_ViewTexture;}
        //temp
        int m_Width, m_Height;
    private:
        RenderTexture m_ViewTexture;
    };
}
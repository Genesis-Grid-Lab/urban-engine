#pragma once

#include "Core/Config.h"

namespace UE {

    class Framebuffer{
    public:
        Framebuffer(int width, int height);
        ~Framebuffer();

        void Resize();
        void Unbind();
        void Bind();

        RenderTexture &GetTexture() { return m_ViewTexture;}
    private:
        int m_Width, m_Height;
        RenderTexture m_ViewTexture;
    };
}
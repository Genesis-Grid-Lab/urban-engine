#pragma once

#include "Renderer/Texture.h"

typedef unsigned int GLenum;

namespace UE {

    class  OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(unsigned int id);
        OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(uint32_t width, uint32_t height, GLenum internalFormat, GLenum dataFormat);
        OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(Ref<Framebuffer>& buffer);
        virtual ~OpenGLTexture2D() override;

        virtual uint32_t GetWidth() const override { return m_Width;  }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetRendererID() const override { return m_RendererID; }
        
        virtual void SetData(void* data, uint32_t size) override;

        virtual void Bind(uint32_t slot = 0) const override;

        virtual bool IsLoaded() const override { return m_IsLoaded; }

        virtual bool operator==(const Texture& other) const override
        {
            return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
        }
		std::string m_Path;
		std::string type;
	private:
		bool m_IsLoaded = false;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};
}
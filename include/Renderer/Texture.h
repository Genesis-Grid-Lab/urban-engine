#pragma once
#include "Config.h"
#include "Framebuffer.h"

typedef unsigned int GLenum;

namespace UE {

	class UE_API Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool IsLoaded() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class UE_API Texture2D : public Texture
	{
	public:
        Texture2D(uint32_t width, uint32_t height);
		Texture2D(uint32_t width, uint32_t height, GLenum internalFormat, GLenum dataFormat);
        Texture2D(const std::string& path);
		Texture2D(Ref<Framebuffer>& buffer);
        virtual ~Texture2D();

        virtual uint32_t GetWidth() const override { return m_Width;  }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetRendererID() const override { return m_RendererID; }
        
        virtual void SetData(void* data, uint32_t size) override;

        virtual void Bind(uint32_t slot = 0) const override;

        virtual bool IsLoaded() const override { return m_IsLoaded; }

        virtual bool operator==(const Texture& other) const override
        {
            return m_RendererID == ((Texture2D&)other).m_RendererID;
        }
	private:
		std::string m_Path;
		bool m_IsLoaded = false;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};

}

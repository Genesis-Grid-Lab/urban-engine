#pragma once
#include "Config.h"
#include "Framebuffer.h"

typedef unsigned int GLenum;

namespace UE {

	class  Texture
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

	class  Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(unsigned int id);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, GLenum internalFormat, GLenum dataFormat);
		static Ref<Texture2D> Create(Ref<Framebuffer>& buffer);
		static Ref<Texture2D> Create(const std::string& path);
	};

}

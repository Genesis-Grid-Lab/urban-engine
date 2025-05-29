#include "uepch.h"
#include "Renderer/Framebuffer.h"
#include "Log.h"
#include "Renderer/Renderer.h"
#include "Core/UE_Assert.h"
#include "Platform/OpenGl/OpenGLFramebuffer.h"

namespace UE {
	
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    UE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLFramebuffer>(spec);
		}

		UE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}

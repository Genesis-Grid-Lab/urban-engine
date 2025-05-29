#include "uepch.h"
#include "Renderer/VertexArray.h"
#include "Core/UE_Assert.h"
#include "Platform/OpenGl/OpenGLVertexArray.h"
#include "Renderer/Renderer.h"

namespace UE {

	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    UE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLVertexArray>();
		}

		UE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}

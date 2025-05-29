#include "uepch.h"
#include "Renderer/Skybox.h"
#include "Renderer/Renderer.h"
#include "Core/UE_Assert.h"
#include "Platform/OpenGl/OpenGLSkybox.h"

namespace UE {

    Ref<Skybox> Skybox::Create(const std::vector<std::string>& faces){
        switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    UE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLSkybox>(faces);
		}

		UE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
    }

}

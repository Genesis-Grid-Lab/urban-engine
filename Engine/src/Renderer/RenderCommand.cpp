#include "uepch.h"
#include "Renderer/RenderCommand.h"

namespace UE {

    Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();
}
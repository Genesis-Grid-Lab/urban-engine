#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"

namespace UE {

    void Renderer::Init(){

        RenderCommand::Init();
        Renderer2D::Init();
    }

    void Renderer::Shutdown(){
        Renderer2D::Shutdown();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

    // void Renderer::Begin(Camera& camera)
	// {
	// 	// s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	// }

	// void Renderer::End()
	// {
	// }

    // void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	// {
	// 	// shader->use();
	// 	// shader->setMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
	// 	// shader->setMat4("u_Transform", transform);

	// 	// vertexArray->Bind();
	// 	// RenderCommand::DrawIndexed(vertexArray);
	// }
}
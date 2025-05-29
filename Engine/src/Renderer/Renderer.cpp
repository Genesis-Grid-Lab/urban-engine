#include "uepch.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Renderer3D.h"

namespace UE {

    void Renderer::Init(){
		UE_PROFILE_FUNCTION();
        RenderCommand::Init();
        Renderer2D::Init();
		Renderer3D::Init();
    }

    void Renderer::Shutdown(){
        Renderer2D::Shutdown();
		Renderer3D::Shutdown();
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
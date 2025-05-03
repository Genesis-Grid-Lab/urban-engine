#pragma once

#include "RenderCommand.h"
// #include "Camera.h"
// #include "Shader.h"

namespace UE {

    class UE_API Renderer{
    public:
        static void Init();
        static void Shutdown();
        
        static void OnWindowResize(uint32_t width, uint32_t height);

        // static void Begin(Camera& camera);
		// static void End();

		// static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));
    };
}
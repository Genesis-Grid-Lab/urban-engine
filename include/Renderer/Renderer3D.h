#pragma once

#include "Core/Config.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Animation/Animation.h"

namespace UE {

    class UE_API Renderer3D{
    public:
        static void Init();
        static void Shutdown();

        static void BeginCamera(const Camera3D& camera);
        static void EndCamera();

        static void RenderLight(const glm::vec3& pos);

        static void DrawModel(const Ref<Model>& model,const glm::mat4& transform, const glm::vec3& color = glm::vec3(1), int entityID = -1);
        static void DrawModel(const Ref<Model>& model,const glm::vec3& position,const glm::vec3& size = glm::vec3(1.0f), const glm::vec3& color = glm::vec3(1));

        static void DrawCube(const glm::mat4& transform, const glm::vec3& color = glm::vec3(1), int entityID = -1);
        static void DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color = glm::vec3(1));

        static void DrawScreen(Ref<Framebuffer>& buffer);

        static void RunAnimation(Ref<Animation> animation, float ts);

        static Ref<Shader>& GetShader();
    private:
        
    };
}
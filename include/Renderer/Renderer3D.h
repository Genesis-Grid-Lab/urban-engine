#pragma once

#include "Core/Config.h"
#include "Shader.h"
#include "Camera.h"
#include "EditorCamera.h"
#include "Model.h"
#include "Animation/Animation.h"
#include "Scene/Components.h"
#include "Scene/SceneCamera.h"

namespace UE {

    class  Renderer3D{
    public:
        static void Init();
        static void Shutdown();

        static void BeginCamera(const Camera& camera);
        static void BeginCamera(const Camera& camera, const TransformComponent& tc);
        static void BeginCamera(const EditorCamera& camera);  
        static void EndCamera();

        static void RenderLight(const glm::vec3& pos);

        static void DrawModel(const Ref<Model>& model,const glm::mat4& transform, const glm::vec3& color = glm::vec3(1), const float transparancy = 1.0f, int entityID = -1);
        static void DrawModel(const Ref<Model>& model,const glm::vec3& position,const glm::vec3& size = glm::vec3(1.0f), const glm::vec3& color = glm::vec3(1), const float transparancy = 1.0f);

        static void DrawCube(const glm::mat4& transform, const glm::vec3& color = glm::vec3(1), const float transparancy = 1.0f, int entityID = -1);
        static void DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color = glm::vec3(1), const float transparancy = 1.0f);
        static void DrawSphere(const glm::mat4& transform, const glm::vec3& color, float transparancy = 1.0f, int entityID = -1);
        static void DrawSphere(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& color = glm::vec3(1), float transparancy = 1.0f);
        static void DrawSphere(const glm::vec3& position, float radius, const glm::vec3& color = glm::vec3(1), float transparancy = 1.0f, int entityID = -1);
        static void SetEntity(int entityID);

        static void DrawWireCube(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color = glm::vec3(1), const float transparancy = 1.0f);
        static void DrawWireSphere(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& color = glm::vec3(1), float transparancy = 1.0f);
        static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);
        static void DrawCameraFrustum(const SceneCamera& cam);
        
        static Ref<Mesh> GetCubeMesh();

        static void DrawScreen(Ref<Framebuffer>& buffer);

        static void RunAnimation(Ref<Animation> animation, float ts);

        static Ref<Shader>& GetShader();
    private:
        
    };
}
#pragma once

#include "Core/Config.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

namespace UE {

    class UE_API Renderer3D{
    public:
        static void Init();
        static void Shutdown();

        static void BeginCamera(const Camera3D& camera);
        static void EndCamera();

        static void DrawModel(const Ref<Model>& model,const glm::vec3& position,const glm::vec3& size = glm::vec3(1.0f));

    private:
        
    };
}
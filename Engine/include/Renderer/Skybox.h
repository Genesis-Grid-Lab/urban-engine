#pragma once

#include "Renderer/Mesh.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/Camera.h"

namespace UE {

    class Skybox {
    public:
        virtual ~Skybox() = default;
        static Ref<Skybox> Create(const std::vector<std::string>& faces); // expects 6 image paths
        virtual void Draw(const Ref<Shader>& shader, const glm::mat4& view, const glm::mat4& projection) = 0;

    };

}

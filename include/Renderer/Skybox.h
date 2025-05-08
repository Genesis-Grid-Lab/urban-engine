#pragma once

#include "Renderer/Mesh.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/Camera.h"

namespace UE {

    class Skybox {
    public:
        Skybox(const std::vector<std::string>& faces); // expects 6 image paths
        void Draw(const Ref<Shader>& shader, const glm::mat4& view, const glm::mat4& projection);

    private:
        Ref<Mesh> m_SkyboxMesh;
        uint32_t m_CubemapID;

        uint32_t LoadCubemap(const std::vector<std::string>& faces);
        Ref<Mesh> CreateCubeMesh();
    };

}

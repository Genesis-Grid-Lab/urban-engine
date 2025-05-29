#pragma once

#include "Renderer/Skybox.h"

namespace UE {

    class OpenGLSkybox : public Skybox {
    public:
        OpenGLSkybox(const std::vector<std::string>& faces); // expects 6 image paths
        virtual ~OpenGLSkybox() override;
        virtual void Draw(const Ref<Shader>& shader, const glm::mat4& view, const glm::mat4& projection) override;

    private:
        Ref<Mesh> m_SkyboxMesh;
        uint32_t m_CubemapID;

        uint32_t LoadCubemap(const std::vector<std::string>& faces);
        Ref<Mesh> CreateCubeMesh();
    };

}

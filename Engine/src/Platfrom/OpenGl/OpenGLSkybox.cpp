#include "uepch.h"
#include "Platform/OpenGl/OpenGLSkybox.h"
#include <glad/glad.h>
#include <stb_image.h>

namespace UE {

    OpenGLSkybox::OpenGLSkybox(const std::vector<std::string>& faces)
    {
        m_CubemapID = LoadCubemap(faces);
        m_SkyboxMesh = CreateCubeMesh();
    }

    OpenGLSkybox::~OpenGLSkybox(){
        
    }

    void OpenGLSkybox::Draw(const Ref<Shader>& shader, const glm::mat4& view, const glm::mat4& projection)
    {
        glDepthFunc(GL_LEQUAL);
        shader->Bind();

        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
        shader->SetMat4("u_View", viewNoTranslation);
        shader->SetMat4("u_Projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapID);
        shader->SetInt("u_Skybox", 0);

        m_SkyboxMesh->Draw(shader);

        glDepthFunc(GL_LESS);
    }

    Ref<Mesh> OpenGLSkybox::CreateCubeMesh()
    {
        std::vector<Vertex> vertices = {
            // positions only (normals and texcoords unused for skybox)
            {{-1.0f,  1.0f, -1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}},
            {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{-1.0f,  1.0f, -1.0f}},

            {{-1.0f, -1.0f,  1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{-1.0f,  1.0f, -1.0f}},
            {{-1.0f,  1.0f, -1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{-1.0f, -1.0f,  1.0f}},

            {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
            {{ 1.0f,  1.0f,  1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}},

            {{-1.0f, -1.0f,  1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
            {{ 1.0f,  1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{-1.0f, -1.0f,  1.0f}},

            {{-1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
            {{ 1.0f,  1.0f,  1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{-1.0f,  1.0f, -1.0f}},

            {{-1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f, -1.0f}},
            {{ 1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}
        };

        std::vector<uint32_t> indices;
        for (uint32_t i = 0; i < 36; ++i)
            indices.push_back(i);

        std::vector<TextureMesh> empty;
        return CreateRef<Mesh>(vertices, indices, empty);
    }

    uint32_t OpenGLSkybox::LoadCubemap(const std::vector<std::string>& faces)
    {
        uint32_t textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (uint32_t i = 0; i < faces.size(); ++i) {
            stbi_set_flip_vertically_on_load(false);
            unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            } else {
                UE_CORE_WARN("Cubemap texture failed to load at path: {0}", faces[i]);
                stbi_image_free(data);
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }

}

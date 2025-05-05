#include "Renderer/Mesh.h"
#include "Renderer/RenderCommand.h"
//temp
#include <glad/glad.h>
namespace UE {
    
    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<Texture2D>& textures)
        :m_Vertices(vertices), m_Indices(indices), m_Textures(textures)
    {
        setupMesh();     
    }


    void Mesh::setupMesh(){

        m_VertexArray = CreateRef<VertexArray>();

        std::vector<float> flatData;
        for (const auto& v : m_Vertices) {
            flatData.push_back(v.Position.x);
            flatData.push_back(v.Position.y);
            flatData.push_back(v.Position.z);
            flatData.push_back(v.Normal.x);
            flatData.push_back(v.Normal.y);
            flatData.push_back(v.Normal.z);
            flatData.push_back(v.TexCoords.x);
            flatData.push_back(v.TexCoords.y);
        }

        UE_CORE_INFO("FlatData {0}", flatData.size());
        
        Ref<VertexBuffer> vertexBuffer = CreateRef<VertexBuffer>(flatData.data(), (uint32_t)flatData.size() * sizeof(float));
        vertexBuffer->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float2, "a_TexCoord"}
        });

        Ref<IndexBuffer> indexBuffer = CreateRef<IndexBuffer>(m_Indices.data(), m_Indices.size());

        m_VertexArray->AddVertexBuffer(vertexBuffer);
        m_VertexArray->SetIndexBuffer(indexBuffer);
        
    }

    void Mesh::Draw(Ref<Shader>& shader){

        shader->Bind();
        uint32_t textureUnit = 0;
        
        for(auto& tex : m_Textures){
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            shader->SetInt("u_DiffuseTexture", textureUnit);
            // tex.Bind(textureUnit);
            glBindTexture(GL_TEXTURE_2D, tex.GetRendererID());
            textureUnit++;
        }                          
        
        RenderCommand::DrawIndexed(m_VertexArray);
        glActiveTexture(GL_TEXTURE0);
    }
}
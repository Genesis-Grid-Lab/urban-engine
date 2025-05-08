#include "Renderer/Mesh.h"
#include "Renderer/RenderCommand.h"
#include "Core/Log.h"
//temp
#include <glad/glad.h>
using namespace std;
namespace UE {
    
    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<TextureMesh>& textures)
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
            flatData.push_back(v.Tangent.x);
            flatData.push_back(v.Tangent.y);
            flatData.push_back(v.Tangent.z);
            flatData.push_back(v.Bitangent.x);
            flatData.push_back(v.Bitangent.y);
            flatData.push_back(v.Bitangent.z);
        }

        UE_CORE_INFO("FlatData {0}", flatData.size());
        
        Ref<VertexBuffer> vertexBuffer = CreateRef<VertexBuffer>(flatData.data(), (uint32_t)flatData.size() * sizeof(float));
        vertexBuffer->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float3, "a_Tangent"},
            {ShaderDataType::Float3, "a_Bitangent"}
        });

        Ref<IndexBuffer> indexBuffer = CreateRef<IndexBuffer>(m_Indices.data(), m_Indices.size());

        m_VertexArray->AddVertexBuffer(vertexBuffer);
        m_VertexArray->SetIndexBuffer(indexBuffer);        
        
    }

    void Mesh::Draw(const Ref<Shader>& shader){

        // bind appropriate textures
        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
        uint32_t textureUnit = 0;
        bool hasNormalMap = false;
        bool hasDiffuseMap = false;
        shader->Bind();
        
        for(auto& tex : m_Textures){
            if(!m_Textures.empty()){
                glActiveTexture(GL_TEXTURE0 + textureUnit);
                // retrieve texture number (the N in diffuse_textureN)
                string number;
                string name = tex.type;
                if(name == "texture_diffuse"){
                    number = std::to_string(diffuseNr++);
                    hasDiffuseMap = true;
                }
                else if(name == "texture_specular")
                    number = std::to_string(specularNr++); // transfer unsigned int to string
                else if(name == "texture_normal"){
                    number = std::to_string(normalNr++); // transfer unsigned int to string
                    hasNormalMap = true;
                }
                else if(name == "texture_height")
                    number = std::to_string(heightNr++); // transfer unsigned int to string
                shader->SetInt((name + number), textureUnit); 
                shader->SetInt("u_UseNormalMap", hasNormalMap);
                shader->SetInt("u_UseDiffuseMap", hasDiffuseMap);
                // tex.Bind(textureUnit);
                glBindTexture(GL_TEXTURE_2D, tex.id);
                textureUnit++;
            }
        }             
        
        m_VertexArray->Bind();        
        glActiveTexture(GL_TEXTURE0);
        RenderCommand::DrawIndexed(m_VertexArray);
    }

   
}
#include "uepch.h"
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

        m_VertexArray = VertexArray::Create();
        
        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));
        vertexBuffer->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float3, "a_Tangent"},
            {ShaderDataType::Float3, "a_Bitangent"},
            {ShaderDataType::Int4, "a_BoneIds"},
            {ShaderDataType::Float4, "a_Weights"},
            {ShaderDataType::Int, "a_EntityID"}
        });

        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size());

        m_VertexArray->AddVertexBuffer(vertexBuffer);
        m_VertexArray->SetIndexBuffer(indexBuffer);  
#if 0        
        for (const auto& vertex : m_Vertices) {
            std::cout << "IDs: " << vertex.m_BoneIDs[0] << ", " << vertex.m_BoneIDs[1] << ", " 
                      << vertex.m_BoneIDs[2] << ", " << vertex.m_BoneIDs[3] << std::endl;
            std::cout << "Weights: " << vertex.m_Weights[0] << ", " << vertex.m_Weights[1] << ", " 
                      << vertex.m_Weights[2] << ", " << vertex.m_Weights[3] << std::endl;
        }
#endif    
        
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
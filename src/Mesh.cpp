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

        // m_VertexArray = CreateRef<VertexArray>();

        // std::vector<float> flatData;
        // for (const auto& v : m_Vertices) {
        //     flatData.push_back(v.Position.x);
        //     flatData.push_back(v.Position.y);
        //     flatData.push_back(v.Position.z);
        //     flatData.push_back(v.Normal.x);
        //     flatData.push_back(v.Normal.y);
        //     flatData.push_back(v.Normal.z);
        //     flatData.push_back(v.TexCoords.x);
        //     flatData.push_back(v.TexCoords.y);
        // }

        // UE_CORE_INFO("FlatData {0}", flatData.size());
        
        // Ref<VertexBuffer> vertexBuffer = CreateRef<VertexBuffer>(flatData.data(), (uint32_t)flatData.size() * sizeof(float));
        // vertexBuffer->SetLayout({
        //     {ShaderDataType::Float3, "a_Position"},
        //     {ShaderDataType::Float3, "a_Normal"},
        //     {ShaderDataType::Float2, "a_TexCoord"},
        //     {ShaderDataType::Float3, "a_Tangent"}
        //     // {ShaderDataType::Float3, "a_Bitangent"}
        // });

        // Ref<IndexBuffer> indexBuffer = CreateRef<IndexBuffer>(m_Indices.data(), m_Indices.size());

        // m_VertexArray->AddVertexBuffer(vertexBuffer);
        // m_VertexArray->SetIndexBuffer(indexBuffer);

        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        
    }

    void Mesh::Draw(Ref<Shader>& shader){

        // bind appropriate textures
        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
        shader->Bind();
        uint32_t textureUnit = 0;
        
        // for(auto& tex : m_Textures){
        //     glActiveTexture(GL_TEXTURE0 + textureUnit);
        //     shader->SetInt("u_DiffuseTexture", textureUnit);
        //     // tex.Bind(textureUnit);
        //     glBindTexture(GL_TEXTURE_2D, tex.GetRendererID());
        //     textureUnit++;
        // }    
        
        for(unsigned int i = 0; i < m_Textures.size(); i++){
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            string number;
            string name = m_Textures[i].type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++); // transfer unsigned int to string
            else if(name == "texture_normal")
                number = std::to_string(normalNr++); // transfer unsigned int to string
             else if(name == "texture_height")
                number = std::to_string(heightNr++); // transfer unsigned int to string

            // now set the sampler to the correct texture unit            
            shader->SetInt((name + number), i);                     
            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, m_Textures[i].id);
        }
        
        // m_VertexArray->Bind();
        // RenderCommand::DrawIndexed(m_VertexArray);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_Indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }

   
}
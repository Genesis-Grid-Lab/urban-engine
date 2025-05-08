#pragma once

#include "Core/Config.h"
#include "Texture.h"
#include "Shader.h"
#include "VertexArray.h"

#define MAX_BONE_INFLUENCE 4

namespace UE {
    
    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;

        int m_BoneIDs[MAX_BONE_INFLUENCE];
        float m_Weights[MAX_BONE_INFLUENCE];
    };

    struct TextureMesh {
        unsigned int id;
        std::string type;
        std::string path;
    };


    class Mesh {
    public:
        Mesh(const std::vector<Vertex>& vertices,const std::vector<uint32_t>& indices,const std::vector<TextureMesh>& textures);

        void Draw(const Ref<Shader> &shader);

    public:
        //mesh data
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<TextureMesh> m_Textures;
        Ref<VertexArray> m_VertexArray;   
        unsigned int VAO;     
    private:        
        unsigned int VBO, EBO;
        void setupMesh();
    };
    
}
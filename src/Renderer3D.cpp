#include "Renderer/Renderer3D.h"
#include "Renderer/Skybox.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/Animation/Animator.h"
#include "Core/Application.h"
//temp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace UE {
    static Ref<Shader> m_Shader;    
    static Ref<Shader> m_LightShader;
    static Ref<Shader> m_SkyShader;
    static glm::vec3 m_LightPos;      
    static Ref<Model> m_LightModel;   
    static Ref<Skybox> m_Skybox; 
    static Ref<Mesh> s_CubeMesh;    
    static Ref<VertexBuffer> ScreenVertexBuffer;
	static Ref<VertexArray> ScreenVertexArray;
    static Ref<Animator> s_Animator;

    Ref<Mesh> GenerateCubeMesh() {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        glm::vec3 positions[] = {
            {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, // front
            {-0.5f, -0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f}, // back
            {-0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f, -0.5f}, // top
            {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f,  0.5f}, // bottom
            { 0.5f, -0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, // right
            {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f, -0.5f}  // left
        };

        glm::vec3 normals[] = {
            { 0,  0,  1}, { 0,  0, -1}, { 0,  1,  0}, { 0, -1,  0}, { 1,  0,  0}, {-1,  0,  0}
        };

        glm::vec2 uvs[] = {
            {0, 0}, {1, 0}, {1, 1}, {0, 1}
        };

        uint32_t faceIndices[] = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            8, 9,10,10,11, 8,
           12,13,14,14,15,12,
           16,17,18,18,19,16,
           20,21,22,22,23,20
        };

        for (int face = 0; face < 6; ++face) {
            for (int i = 0; i < 4; ++i) {
                Vertex v;
                v.Position = positions[face * 4 + i];
                v.Normal = normals[face];
                v.TexCoords = uvs[i];
                v.Tangent = glm::vec3(1, 0, 0);     // Placeholder
                v.Bitangent = glm::vec3(0, 1, 0);   // Placeholder
                vertices.push_back(v);
            }
        }

        for (int i = 0; i < 36; ++i)
            indices.push_back(faceIndices[i]);

        std::vector<TextureMesh> noTextures; // Can be replaced later

        return CreateRef<Mesh>(vertices, indices, noTextures);
    }

    Ref<Shader>& Renderer3D::GetShader(){
        return m_Shader;
    }
    void Renderer3D::Init(){
        // s_ScreenShader = CreateRef<Shader>("Data/Shaders/Screen.glsl");
        m_Shader = CreateRef<Shader>("Data/Shaders/model.glsl");
        m_LightShader = CreateRef<Shader>("Data/Shaders/BasicLight.glsl");
        m_SkyShader = CreateRef<Shader>("Data/Shaders/skybox.glsl");
        m_LightModel = CreateRef<Model>("Resources/cube.fbx");    
        std::vector<std::string> faces = {
            "Resources/skybox/right.jpg", "Resources/skybox/left.jpg", "Resources/skybox/top.jpg",
            "Resources/skybox/bottom.jpg", "Resources/skybox/front.jpg", "Resources/skybox/back.jpg"
        };
        
        m_Skybox = CreateRef<Skybox>(faces);

        s_CubeMesh = GenerateCubeMesh();

        ScreenVertexArray = CreateRef<VertexArray>();

        float quadVertices[] = {
			// positions   // texCoords
			-1.0f, -1.0f,  0.0f, 0.0f,
			1.0f, -1.0f,  1.0f, 0.0f,
			1.0f,  1.0f,  1.0f, 1.0f,
			-1.0f,  1.0f,  0.0f, 1.0f
		};

        uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

        ScreenVertexBuffer = CreateRef<VertexBuffer>(quadVertices, sizeof(quadVertices));
		ScreenVertexBuffer->SetLayout({
			{ ShaderDataType::Float2, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		
		const auto& ib = CreateRef<IndexBuffer>(indices, 6);
		
		ScreenVertexArray->AddVertexBuffer(ScreenVertexBuffer);
		ScreenVertexArray->SetIndexBuffer(ib);

        s_Animator = CreateRef<Animator>();
    }

    void Renderer3D::Shutdown(){}

    void Renderer3D::BeginCamera(const Camera3D& camera){
        m_Shader->Bind();
        m_Shader->SetMat4("u_View", camera.GetViewMatrix());
        m_Shader->SetMat4("u_Projection", camera.GetProjectionMatrix());
        m_Shader->SetFloat3("u_ViewPos", camera.GetPosition());
        
        m_Shader->SetFloat3("u_LightPos", m_LightPos);             
        m_Shader->SetInt("u_EntityID", -1);
        m_LightShader->Bind();
        m_LightShader->SetMat4("u_View", camera.GetViewMatrix());
        m_LightShader->SetMat4("u_Projection", camera.GetProjectionMatrix());    
        
        m_SkyShader->Bind();
        m_Skybox->Draw(m_SkyShader, camera.GetViewMatrix(), camera.GetProjectionMatrix());
    }

    void Renderer3D::EndCamera(){}

    void Renderer3D::RenderLight(const glm::vec3& pos){
        m_LightShader->Bind();
        m_LightPos = pos;  
        glm::mat4 imodel = glm::mat4(1.0f);
        imodel = glm::translate(imodel, pos);
        imodel = glm::rotate(imodel, glm::radians(0.0f), glm::vec3(0, 1, 0));
        imodel = glm::scale(imodel, glm::vec3(1));        

        m_LightShader->SetMat4("u_Model", imodel);     
                

        m_LightModel->Draw(m_LightShader);
    }

    void Renderer3D::DrawModel(const Ref<Model>& model,const glm::mat4& transform, const glm::vec3& color, int entityID){
        m_Shader->Bind();
        m_Shader->SetFloat3("u_Color", color);  
        m_Shader->SetMat4("u_Model", transform);  
        for(auto& mesh : model->GetMeshes()){
            for(auto& vertex : mesh->m_Vertices){
                vertex.EntityID = entityID;
            }
        }        

        m_Shader->SetInt("u_EntityID", entityID);
        model->Draw(m_Shader);
    }

    void Renderer3D::DrawModel(const Ref<Model>& model,const glm::vec3& position,const glm::vec3& size, const glm::vec3& color){
        
        glm::mat4 imodel = glm::mat4(1.0f);
        imodel = glm::translate(imodel, position);
        imodel = glm::rotate(imodel, glm::radians(0.0f), glm::vec3(0, 1, 0));
        imodel = glm::scale(imodel, size);

        DrawModel(model, imodel, color);

    }


    void Renderer3D::DrawCube(const glm::mat4& transform, const glm::vec3& color, int entityID){
        m_Shader->Bind();
        m_Shader->SetMat4("u_Model", transform);
        m_Shader->SetFloat3("u_Color", color); 
        m_Shader->SetInt("u_EntityID", entityID);
        s_CubeMesh->Draw(m_Shader);
    }
    
    void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                            * glm::scale(glm::mat4(1.0f), size);
        DrawCube(transform, color);
    }

    void Renderer3D::DrawScreen(Ref<Framebuffer>& buffer){
        // Bind default framebuffer (screen)		
		glViewport(0, 0, buffer->GetSpecification().Width, buffer->GetSpecification().Height);	
	
		// Bind blit shader
		Application::Get().GetScreenShader()->Bind();
	
		// Bind source texture
		// glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buffer->GetColorAttachmentRendererID());
		Application::Get().GetScreenShader()->SetInt("screenTexture", 0);
	
		// Draw fullscreen quad		
	
		ScreenVertexArray->Bind();
		RenderCommand::DrawIndexed(ScreenVertexArray);				
    }

    void Renderer3D::RunAnimation(Ref<Animation> animation, float ts){
        if(s_Animator->GetCurrentAnimation() != animation){
            s_Animator->PlayAnimation(animation);
        }

        s_Animator->UpdateAnimation(ts);

        auto finalBones = s_Animator->GetFinalBoneMatrices();
        for (int i = 0; i < finalBones.size(); ++i)
        {
            std::string uniformName = "u_FinalBonesMatrices[" + std::to_string(i) + "]";
            m_Shader->SetMat4(uniformName, finalBones[i]);
        }
    }
}
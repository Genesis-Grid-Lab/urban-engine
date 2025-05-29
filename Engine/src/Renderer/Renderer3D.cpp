#include "uepch.h"
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
    static Ref<Shader> m_ShaderSimple;    
    static Ref<Shader> m_LightShader;
    static Ref<Shader> m_SkyShader;
    static glm::vec3 m_LightPos;      
    static Ref<Model> m_LightModel;   
    static Ref<Skybox> m_Skybox; 
    static Ref<Mesh> s_CubeMesh; 
    static Ref<Mesh> s_SphereMesh;   
    static Ref<VertexBuffer> ScreenVertexBuffer;
	static Ref<VertexArray> ScreenVertexArray;
    static Ref<Animator> s_Animator;

    //TODO: remove
    static GLuint lineVAO = 0, lineVBO = 0;

    Ref<Mesh> GenerateSphereMesh(uint32_t sectorCount = 36, uint32_t stackCount = 18) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float x, y, z, xy;                              // vertex position
        float nx, ny, nz, lengthInv = 1.0f;             // vertex normal
        float s, t;                                     // vertex texCoord

        float sectorStep = 2 * glm::pi<float>() / sectorCount;
        float stackStep = glm::pi<float>() / stackCount;
        float sectorAngle, stackAngle;

        for (uint32_t i = 0; i <= stackCount; ++i) {
            stackAngle = glm::pi<float>() / 2 - i * stackStep; // from pi/2 to -pi/2
            xy = cos(stackAngle);                              // r * cos(u)
            z = sin(stackAngle);                               // r * sin(u)

            for (uint32_t j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;

                x = xy * cos(sectorAngle);                     // x = r * cos(u) * cos(v)
                y = xy * sin(sectorAngle);                     // y = r * cos(u) * sin(v)
                nx = x; ny = y; nz = z;

                Vertex v;
                v.Position = glm::vec3(x, y, z);
                v.Normal = glm::normalize(glm::vec3(nx, ny, nz));
                v.TexCoords = glm::vec2((float)j / sectorCount, (float)i / stackCount);
                v.Tangent = glm::vec3(1, 0, 0);    // placeholder
                v.Bitangent = glm::vec3(0, 1, 0);  // placeholder
                vertices.push_back(v);
            }
        }

        // index generation
        for (uint32_t i = 0; i < stackCount; ++i) {
            uint32_t k1 = i * (sectorCount + 1);     // beginning of current stack
            uint32_t k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                if (i != 0)
                    indices.insert(indices.end(), { k1, k2, k1 + 1 });
                if (i != (stackCount - 1))
                    indices.insert(indices.end(), { k1 + 1, k2, k2 + 1 });
            }
        }

        std::vector<TextureMesh> noTextures;
        return CreateRef<Mesh>(vertices, indices, noTextures);
    }

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
        UE_PROFILE_FUNCTION();
        // s_ScreenShader = CreateRef<Shader>("Data/Shaders/Screen.glsl");
        m_Shader = Shader::Create("Data/Shaders/model.glsl");
        m_ShaderSimple = Shader::Create("Data/Shaders/Simplemodel.glsl");
        m_LightShader = Shader::Create("Data/Shaders/BasicLight.glsl");
        m_SkyShader = Shader::Create("Data/Shaders/skybox.glsl");
        m_LightModel = CreateRef<Model>("Resources/cube.fbx");    
        std::vector<std::string> faces = {
            "Resources/skybox/right.jpg", "Resources/skybox/left.jpg", "Resources/skybox/top.jpg",
            "Resources/skybox/bottom.jpg", "Resources/skybox/front.jpg", "Resources/skybox/back.jpg"
        };
        
        m_Skybox = Skybox::Create(faces);

        s_CubeMesh = GenerateCubeMesh();
        s_SphereMesh = GenerateSphereMesh();

        ScreenVertexArray = VertexArray::Create();

        float quadVertices[] = {
			// positions   // texCoords
			-1.0f, -1.0f,  0.0f, 0.0f,
			1.0f, -1.0f,  1.0f, 0.0f,
			1.0f,  1.0f,  1.0f, 1.0f,
			-1.0f,  1.0f,  0.0f, 1.0f
		};

        uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

        ScreenVertexBuffer = VertexBuffer::Create(quadVertices, sizeof(quadVertices));
		ScreenVertexBuffer->SetLayout({
			{ ShaderDataType::Float2, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		
		const auto& ib = IndexBuffer::Create(indices, 6);
		
		ScreenVertexArray->AddVertexBuffer(ScreenVertexBuffer);
		ScreenVertexArray->SetIndexBuffer(ib);

        s_Animator = CreateRef<Animator>();

        glGenVertexArrays(1, &lineVAO);
		glGenBuffers(1, &lineVBO);
		glBindVertexArray(lineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glBindVertexArray(0);
    }

    void Renderer3D::Shutdown(){
        UE_PROFILE_FUNCTION();
    }

    void Renderer3D::BeginCamera(const Camera& camera){
        UE_PROFILE_FUNCTION();
        m_Shader->Bind();
        m_Shader->SetMat4("u_View", camera.GetViewMatrix());
        m_Shader->SetMat4("u_Projection", camera.GetProjectionMatrix());
        m_Shader->SetFloat3("u_ViewPos", camera.GetPosition());        
        
        m_Shader->SetFloat3("u_LightPos", m_LightPos);                     
        m_LightShader->Bind();
        m_LightShader->SetMat4("u_View", camera.GetViewMatrix());
        m_LightShader->SetMat4("u_Projection", camera.GetProjectionMatrix()); 
           
        m_ShaderSimple->Bind();
        m_ShaderSimple->SetMat4("u_View", camera.GetViewMatrix());
        m_ShaderSimple->SetMat4("u_Projection", camera.GetProjectionMatrix());
        m_ShaderSimple->SetFloat3("u_ViewPos", camera.GetPosition());        
        
        m_ShaderSimple->SetFloat3("u_LightPos", m_LightPos); 

        m_SkyShader->Bind();
        m_Skybox->Draw(m_SkyShader, camera.GetViewMatrix(), camera.GetProjectionMatrix());
    }

    void Renderer3D::BeginCamera(const Camera& camera, const TransformComponent& tc){
        UE_PROFILE_FUNCTION();
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Translation);
        transform = glm::rotate(transform, tc.Rotation.x, glm::vec3(1, 0, 0)); // Pitch
        transform = glm::rotate(transform, tc.Rotation.y, glm::vec3(0, 1, 0)); // Yaw
        transform = glm::rotate(transform, tc.Rotation.z, glm::vec3(0, 0, 1)); // Roll

        m_Shader->Bind();
        m_Shader->SetMat4("u_View", glm::inverse(transform));
        m_Shader->SetMat4("u_Projection", camera.GetProjectionMatrix());
        m_Shader->SetFloat3("u_ViewPos", tc.Translation);        
        
        m_Shader->SetFloat3("u_LightPos", m_LightPos);                     
        m_LightShader->Bind();
        m_LightShader->SetMat4("u_View", glm::inverse(transform));
        m_LightShader->SetMat4("u_Projection", camera.GetProjectionMatrix());   
        
        m_ShaderSimple->Bind();
        m_ShaderSimple->SetMat4("u_View", glm::inverse(transform));
        m_ShaderSimple->SetMat4("u_Projection", camera.GetProjectionMatrix());
        m_ShaderSimple->SetFloat3("u_ViewPos", tc.Translation);        
        
        m_ShaderSimple->SetFloat3("u_LightPos", m_LightPos);
        
        m_SkyShader->Bind();
        m_Skybox->Draw(m_SkyShader, glm::inverse(transform), camera.GetProjectionMatrix());
    }

    void Renderer3D::BeginCamera(const EditorCamera& camera)
	{
        UE_PROFILE_FUNCTION();
		m_Shader->Bind();
        m_Shader->SetMat4("u_View", camera.GetViewMatrix());
        m_Shader->SetMat4("u_Projection", camera.GetProjectionMatrix());
        m_Shader->SetFloat3("u_ViewPos", camera.GetPosition());
        
        m_Shader->SetFloat3("u_LightPos", m_LightPos);             
        m_Shader->SetInt("u_EntityID", -1);
        m_LightShader->Bind();
        m_LightShader->SetMat4("u_View", camera.GetViewMatrix());
        m_LightShader->SetMat4("u_Projection", camera.GetProjectionMatrix());  
        
        m_ShaderSimple->Bind();
        m_ShaderSimple->SetMat4("u_View", camera.GetViewMatrix());
        m_ShaderSimple->SetMat4("u_Projection", camera.GetProjectionMatrix());
        m_ShaderSimple->SetFloat3("u_ViewPos", camera.GetPosition());
        
        m_ShaderSimple->SetFloat3("u_LightPos", m_LightPos);             
        m_ShaderSimple->SetInt("u_EntityID", -1);
        
        m_SkyShader->Bind();
        m_Skybox->Draw(m_SkyShader, camera.GetViewMatrix(), camera.GetProjectionMatrix());
	}

    void Renderer3D::EndCamera(){
        UE_PROFILE_FUNCTION();
    }

    void Renderer3D::RenderLight(const glm::vec3& pos){
        UE_PROFILE_FUNCTION();
        m_LightShader->Bind();
        m_LightPos = pos;  
        glm::mat4 imodel = glm::mat4(1.0f);
        imodel = glm::translate(imodel, pos);
        imodel = glm::rotate(imodel, glm::radians(0.0f), glm::vec3(0, 1, 0));
        imodel = glm::scale(imodel, glm::vec3(1));        

        m_LightShader->SetMat4("u_Model", imodel);     
                

        m_LightModel->Draw(m_LightShader);
    }

    void Renderer3D::DrawModel(const Ref<Model>& model,const glm::mat4& transform, const glm::vec3& color, const float transparancy, int entityID){
        UE_PROFILE_FUNCTION();
        m_Shader->Bind();
        m_Shader->SetFloat3("u_Color", color);  
        m_Shader->SetMat4("u_Model", transform);  
        m_Shader->SetFloat("u_Transparancy", 1.0f);
        for(auto& mesh : model->GetMeshes()){
            for(auto& vertex : mesh->m_Vertices){
                vertex.EntityID = entityID;
            }
        }        

        m_Shader->SetInt("u_EntityID", entityID);
        model->Draw(m_Shader);
        m_Shader->Unbind();
    }

    void Renderer3D::SetEntity(int entityID){
        m_Shader->Bind();
        m_Shader->SetInt("u_EntityID", entityID);
        m_Shader->Unbind();
    }

    void Renderer3D::DrawModel(const Ref<Model>& model,const glm::vec3& position,const glm::vec3& size, const glm::vec3& color, const float transparancy){        
        glm::mat4 imodel = glm::mat4(1.0f);
        imodel = glm::translate(imodel, position);
        imodel = glm::rotate(imodel, glm::radians(0.0f), glm::vec3(0, 1, 0));
        imodel = glm::scale(imodel, size);

        DrawModel(model, imodel, color, transparancy);

    }

    void Renderer3D::DrawCube(const glm::mat4& transform, const glm::vec3& color, const float transparancy, int entityID){
        UE_PROFILE_FUNCTION();
        m_ShaderSimple->Bind();
        m_ShaderSimple->SetMat4("u_Model", transform);
        m_ShaderSimple->SetFloat3("u_Color", color); 
        m_ShaderSimple->SetInt("u_EntityID", entityID);
        m_ShaderSimple->SetFloat("u_Transparancy", transparancy);        
        s_CubeMesh->Draw(m_ShaderSimple);
        m_ShaderSimple->Unbind();
    }

    Ref<Mesh> Renderer3D::GetCubeMesh(){
        return s_CubeMesh;
    }
    
    void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, const float transparancy) {
        
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                            * glm::scale(glm::mat4(1.0f), size);        
        DrawCube(transform, color, transparancy);
    }

    void Renderer3D::DrawSphere(const glm::mat4& transform, const glm::vec3& color, float transparancy, int entityID) {
        UE_PROFILE_FUNCTION();
        m_ShaderSimple->Bind();
        m_ShaderSimple->SetMat4("u_Model", transform);
        m_ShaderSimple->SetFloat3("u_Color", color); 
        m_ShaderSimple->SetInt("u_EntityID", entityID);
        m_ShaderSimple->SetFloat("u_Transparancy", transparancy);
        s_SphereMesh->Draw(m_ShaderSimple);
        m_ShaderSimple->Unbind();
    }

    void Renderer3D::DrawSphere(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& color, float transparancy) {
        
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                            * glm::scale(glm::mat4(1.0f), scale);
        DrawSphere(transform, color, transparancy);
    }

    void Renderer3D::DrawSphere(const glm::vec3& position, float radius, const glm::vec3& color, float transparancy, int entityID) {
        
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                            * glm::scale(glm::mat4(1.0f), glm::vec3(radius));
        DrawSphere(transform, color, transparancy, entityID);
    }

    void Renderer3D::DrawWireCube(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, const float transparancy){
        UE_PROFILE_FUNCTION();
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        DrawCube(position, size, color, transparancy);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void Renderer3D::DrawWireSphere(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& color, float transparancy){
        UE_PROFILE_FUNCTION();
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        DrawSphere(position, scale, color, transparancy);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void Renderer3D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color) {
        UE_PROFILE_FUNCTION();
		glm::vec3 points[2] = { p0, p1 };
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);

		glBindVertexArray(lineVAO);		
		
		m_ShaderSimple->Bind();		
        // m_ShaderSimple->SetMat4("u_Model", glm::mat4(1.0f));
		m_ShaderSimple->SetFloat4("u_Color", color);

		glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
	}

    void Renderer3D::DrawCameraFrustum(const UE::SceneCamera& cam)
    {
        UE_PROFILE_FUNCTION();
        using namespace UE;

        glm::vec3 pos = cam.GetPosition();
        glm::vec3 forward = cam.GetForwardDirection();
        glm::vec3 right = cam.GetRightDirection();
        glm::vec3 up = cam.GetUpDirection();

        float nearD = cam.GetPerspectiveNearClip();
        float farD = cam.GetPerspectiveFarClip();
        float fov = cam.GetVerticalFOV();
        float ar = cam.GetAspectRatio();

        float nearH = 2.0f * tan(fov / 2.0f) * nearD;
        float nearW = nearH * ar;
        float farH = 2.0f * tan(fov / 2.0f) * farD;
        float farW = farH * ar;

        glm::vec3 nc = pos + forward * nearD;
        glm::vec3 fc = pos + forward * farD;

        glm::vec3 ntl = nc + (up * nearH / 2.0f) - (right * nearW / 2.0f);
        glm::vec3 ntr = nc + (up * nearH / 2.0f) + (right * nearW / 2.0f);
        glm::vec3 nbl = nc - (up * nearH / 2.0f) - (right * nearW / 2.0f);
        glm::vec3 nbr = nc - (up * nearH / 2.0f) + (right * nearW / 2.0f);

        glm::vec3 ftl = fc + (up * farH / 2.0f) - (right * farW / 2.0f);
        glm::vec3 ftr = fc + (up * farH / 2.0f) + (right * farW / 2.0f);
        glm::vec3 fbl = fc - (up * farH / 2.0f) - (right * farW / 2.0f);
        glm::vec3 fbr = fc - (up * farH / 2.0f) + (right * farW / 2.0f);

        // glm::mat4 vp = cam.GetViewProjection();
        glm::vec4 color = {1, 1, 0, 1}; // Yellow

        Renderer3D::DrawLine(ntl, ntr, color);
        Renderer3D::DrawLine(ntr, nbr, color);
        Renderer3D::DrawLine(nbr, nbl, color);
        Renderer3D::DrawLine(nbl, ntl, color);

        Renderer3D::DrawLine(ftl, ftr, color);
        Renderer3D::DrawLine(ftr, fbr, color);
        Renderer3D::DrawLine(fbr, fbl, color);
        Renderer3D::DrawLine(fbl, ftl, color);

        Renderer3D::DrawLine(ntl, ftl, color);
        Renderer3D::DrawLine(ntr, ftr, color);
        Renderer3D::DrawLine(nbr, fbr, color);
        Renderer3D::DrawLine(nbl, fbl, color);
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
        UE_PROFILE_FUNCTION();
        m_Shader->Bind();
        if(s_Animator->GetCurrentAnimation() != animation){
            s_Animator->PlayAnimation(animation);
            UE_CORE_INFO("[renderer3d]: Running");
        }

        s_Animator->UpdateAnimation(ts);

        auto finalBones = s_Animator->GetFinalBoneMatrices();
        for (int i = 0; i < finalBones.size(); ++i)
        {
            std::string uniformName = "u_FinalBonesMatrices[" + std::to_string(i) + "]";
            m_Shader->SetMat4(uniformName, finalBones[i]);
        }

        m_Shader->Unbind();
    }
}
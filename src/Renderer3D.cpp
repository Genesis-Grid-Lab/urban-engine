#include "Renderer/Renderer3D.h"


namespace UE {
    static Ref<Shader> m_Shader;    
    static glm::vec3 m_LightPos;   
    static bool blinn = true;
    
    static glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	static glm::vec3 pointLightColors[] = {
		glm::vec3(0.8f, 0.6f, 0.5f),
		glm::vec3(0.8f, 0.6f, 0.5f),
		glm::vec3(0.8f, 0.6f, 0.5f),
		glm::vec3(0.8f, 0.6f, 0.5f)
	};

    void Renderer3D::Init(){

        m_Shader = CreateRef<Shader>("Data/Shaders/model.glsl");
        m_LightPos = { 3.0f, 5.0f, 2.0f };        
    }

    void Renderer3D::Shutdown(){}

    void Renderer3D::BeginCamera(const Camera3D& camera){
        m_Shader->Bind();
        m_Shader->SetMat4("u_View", camera.GetViewMatrix());
        m_Shader->SetMat4("u_Projection", camera.GetProjectionMatrix());
        m_Shader->SetFloat3("u_ViewPos", camera.GetPosition());

        m_Shader->SetFloat3("u_LightPos", m_LightPos);
        m_Shader->SetInt("u_Blinn", blinn);
    }

    void Renderer3D::EndCamera(){}

    void Renderer3D::DrawModel(const Ref<Model>& model,const glm::vec3& position,const glm::vec3& size){
        m_Shader->Bind();

        glm::mat4 imodel = glm::mat4(1.0f);
        imodel = glm::translate(imodel, position);
        imodel = glm::rotate(imodel, glm::radians(0.0f), glm::vec3(0, 1, 0));
        imodel = glm::scale(imodel, size);

        m_Shader->SetMat4("u_Model", imodel);     
                

        model->Draw(m_Shader);
    }
}
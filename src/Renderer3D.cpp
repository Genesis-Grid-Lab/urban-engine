#include "Renderer/Renderer3D.h"


namespace UE {
    static Ref<Shader> m_Shader;    
    static glm::vec3 m_LightPos;          

    void Renderer3D::Init(){

        m_Shader = CreateRef<Shader>("Data/Shaders/model.glsl");
        m_LightPos = { 0.5f, 1.0f, 0.3f };        
    }

    void Renderer3D::Shutdown(){}

    void Renderer3D::BeginCamera(const Camera3D& camera){
        m_Shader->Bind();
        m_Shader->SetMat4("u_View", camera.GetViewMatrix());
        m_Shader->SetMat4("u_Projection", camera.GetProjectionMatrix());
        m_Shader->SetFloat3("u_ViewPos", camera.GetPosition());

        m_Shader->SetFloat3("u_LightPos", m_LightPos);        
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
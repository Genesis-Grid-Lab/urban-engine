#pragma once
#include "Core/Config.h"
#include "Core/Timestep.h"
#include <glm/glm.hpp>

namespace UE {
	// Default camera values
	const float Default_YAW         = -90.0f;
	const float Default_PITCH       =  0.0f;
	const float Default_SPEED       =  5.5f;
	const float Default_SENSITIVITY =  0.1f;
	const float Default_ZOOM        =  45.0f;
	
	class  Camera {
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			: m_ProjectionMatrix(projection) {}
		virtual ~Camera() = default;
		void SetPosition(const glm::vec3& position) { *m_Position = position; RecalculateViewMatrix(); }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
		void SetRotation2(const glm::vec3& rotation) { m_Rotation2 = rotation; RecalculateViewMatrix();}

		float GetRotation() const { return m_Rotation; }
		const glm::vec3& GetRotation2() const { return m_Rotation2;}
		const glm::vec3& GetPosition() const { return *m_Position; }
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
		glm::vec3* m_Position = &glm::vec3(0);
	private:
		virtual void RecalculateViewMatrix() {};
	protected:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;
		glm::vec3 m_Rotation2 = {0,0,0};
	private:
		float m_Rotation = 0.0f;
		friend class Camera2D;
		friend class Camera3D;
	};

    class  Camera2D : public Camera {
    public:        
		Camera2D() = default;
        Camera2D(float left, float right, float bottom, float top);
        void SetProjection(float left, float right, float bottom, float top);		   									
	private:				
		void RecalculateViewMatrix();
    };
	
}
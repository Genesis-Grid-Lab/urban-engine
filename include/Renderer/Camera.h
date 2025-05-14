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
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		float GetRotation() const { return m_Rotation; }
		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
	private:
		virtual void RecalculateViewMatrix() {};
	protected:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
	private:
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
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

	class  Camera3D : public Camera {
	public:
		Camera3D() = default;
		Camera3D(const glm::vec2& size, const glm::vec3& position = glm::vec3(0), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
				 const float& yaw = -90.0f, const float& pitch = 0.0f);
		Camera3D(float fov, float aspectRatio, float nearClip, float farClip);
		void SetProjection(float fov, float aspectRatio, float nearClip, float farClip);
		void OnViewportResize(const glm::vec2& aspect);
		void ProcessInputAndMouse(Timestep ts, bool constrainPitch = true);
	private:
		void RecalculateViewMatrix();
	public:
		glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
	private:
		glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_Right;
		glm::vec3 m_WorldUp = m_Up;
		glm::vec2 m_Size;
		float m_Aspect;
		// euler Angles
		float m_Yaw = Default_YAW;
		float m_Pitch = Default_PITCH;

		// camera options
		float m_MovementSpeed = Default_SPEED;
		float m_MouseSensitivity = Default_SENSITIVITY;
		float m_Zoom = Default_ZOOM;
	private:
		bool firstMouse = true;
		float lastX = 0;
		float lastY = 0;
	};
}
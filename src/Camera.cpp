#include "Renderer/Camera.h"
#include "Core/Input.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Core/Log.h"

namespace UE {

	// camera 2d
	Camera2D::Camera2D(float left, float right, float bottom, float top)
	{		
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -10.0f, 10.0f);
		m_ViewMatrix = glm::mat4(1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
	void Camera2D::RecalculateViewMatrix()
	{		
		
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
		glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
		
		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
	
	void Camera2D::SetProjection(float left, float right, float bottom, float top)
	{		
		
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}	
	
	
	// camera 3d
	Camera3D::Camera3D(float fov, float aspectRatio, float nearClip, float farClip){
		m_ProjectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
		m_ViewMatrix = glm::mat4(1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	Camera3D::Camera3D(const glm::vec2& size, const glm::vec3& position, const glm::vec3& up,
				 const float& yaw, const float& pitch)
	{
		m_Position = position;
		m_WorldUp = up;
		m_Yaw = yaw;
		m_Pitch = pitch;
		m_Size = size;
		m_Aspect = size.x / size.y;				
		RecalculateViewMatrix();
	}

	void Camera3D::SetProjection(float fov, float aspectRatio, float nearClip, float farClip)
	{		
		m_ProjectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void Camera3D::RecalculateViewMatrix()
	{		
		
		// glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
		// glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
		
		// m_ViewMatrix = glm::inverse(transform);
		// m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

		 // calculate the new Front vector
		 glm::vec3 front;
		 front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		 front.y = sin(glm::radians(m_Pitch));
		 front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		 m_Front = glm::normalize(front);
		 // also re-calculate the Right and Up vector
		 m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		 m_Up    = glm::normalize(glm::cross(m_Right, m_Front));

		 m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
		 m_ProjectionMatrix = glm::perspective(glm::radians(m_Zoom), m_Aspect, 0.1f, 100.0f);
		 m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void Camera3D::ProcessInputAndMouse(Timestep ts, bool constrainPitch){

		float velocity = m_MovementSpeed * ts;
		if(Input::IsKeyPressed(Key::W)){
            m_Position += m_Front * velocity;
            // m_Position.z -= velocity;
        }
        if(Input::IsKeyPressed(Key::S)){
            m_Position -= m_Front * velocity;       
            // m_Position.z += velocity;       
        }
        if(Input::IsKeyPressed(Key::A)){
            m_Position -= m_Right * velocity;        
            // m_Position.x -= velocity;        
        }
        if(Input::IsKeyPressed(Key::D)){
            m_Position += m_Right * velocity;     
            // m_Position.x += velocity;     
        }
        if(Input::IsKeyPressed(Key::Space)){
            m_Position += m_Up * velocity;     
            // m_Position.y += velocity;     
        }
        if(Input::IsKeyPressed(Key::LeftControl)){
            m_Position -= m_Up * velocity;          
            // m_Position.y -= velocity;          
        }

		if(Input::IsMouseButtonPressed(Mouse::ButtonRight)){
			Input::HideCursor(true);

			float centerX = m_Size.x / 2.0f;
			float centerY = m_Size.y / 2.0f;						

			float mouseX = Input::GetMouseX();
			float mouseY = Input::GetMouseY();
						
			if(firstMouse){
				lastX = mouseX;
				lastY = mouseY;
				firstMouse = false;
			}
			// else{
				float xoffset = (mouseX - lastX);
				float yoffset = (lastY - mouseY);
				lastX = mouseX;
				lastY = mouseY;
				// UE_CORE_WARN("MOUSES {0} {1}", mouseX, lastX);
			// }
			
			m_Yaw += m_MouseSensitivity * xoffset;
			m_Pitch += m_MouseSensitivity * yoffset;

			if (constrainPitch)
			{
				if (m_Pitch > 89.0f)
					m_Pitch = 89.0f;
				if (m_Pitch < -89.0f)
					m_Pitch = -89.0f;
			}

			Input::SetCursorPos(centerX, centerY);
			lastX = centerX;
			lastY = centerY;
		}
		else{
			firstMouse = true;
			Input::HideCursor(false);
		}

		RecalculateViewMatrix();
	}
}
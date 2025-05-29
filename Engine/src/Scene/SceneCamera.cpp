#include "uepch.h"
#include "Scene/SceneCamera.h"
#include "Core/UE_Assert.h"
#include <glm/gtc/matrix_transform.hpp>

namespace UE {

	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_PerspectiveFOV = verticalFOV;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;
		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		UE_CORE_ASSERT(width > 0 && height > 0);
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		if (m_ProjectionType == ProjectionType::Perspective)
		{
			m_ProjectionMatrix = glm::perspective(m_PerspectiveFOV, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
		}
		else
		{
			float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
			float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
			float orthoBottom = -m_OrthographicSize * 0.5f;
			float orthoTop = m_OrthographicSize * 0.5f;

			m_ProjectionMatrix = glm::ortho(orthoLeft, orthoRight,
				orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
		}

		RecalculateView();
		        
	}

	void SceneCamera::RecalculateView(){
		glm::vec3 position;
		glm::vec3 direction;
		
		switch (m_Mode)
		{
			case CameraMode::ThirdPerson:
			{
				glm::vec3 behind = glm::normalize(m_Target - (m_Target + m_Offset));
				position = m_Target + m_Offset;
				direction = glm::normalize(m_Target - position);
				break;
			}

			case CameraMode::FirstPerson:
			{
				position = m_Target;
				direction = glm::normalize(glm::vec3(
					cos(m_Rotation2.y) * cos(m_Rotation2.x),
					sin(m_Rotation2.x),
					sin(m_Rotation2.y) * cos(m_Rotation2.x)
				));
				break;
			}

			case CameraMode::TopDown:
			{
				position = m_Target + glm::vec3(0.0f, m_Offset.y, 0.0f);
				direction = glm::vec3(0.0f, -1.0f, 0.0f); // looking straight down
				break;
			}
		}

		m_ViewMatrix = glm::lookAt(position, position + direction, glm::vec3(0, 1, 0));
		*m_Position = position;
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}

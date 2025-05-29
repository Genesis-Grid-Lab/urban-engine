#pragma once

#include "Renderer/Camera.h"

namespace UE {
	enum class CameraMode { FirstPerson = 0, ThirdPerson = 1, TopDown = 2};

	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };		
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetPerspective(float verticalFOV, float nearClip, float farClip);
		void SetOrthographic(float size, float nearClip, float farClip);

		void SetViewportSize(uint32_t width, uint32_t height);

		float GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
		void SetPerspectiveVerticalFOV(float verticalFov) { m_PerspectiveFOV = verticalFov; RecalculateProjection(); }
		float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }
		float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
		void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }

		float GetOrthographicSize() const { return m_OrthographicSize; }
		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
		float GetOrthographicNearClip() const { return m_OrthographicNear; }
		void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
		float GetOrthographicFarClip() const { return m_OrthographicFar; }
		void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }

		void SetMode(CameraMode mode) { m_Mode = mode; RecalculateView(); }
		void SetTarget(const glm::vec3& target) { m_Target = target; RecalculateView(); }
		void SetOffset(const glm::vec3& offset) { m_Offset = offset; RecalculateView(); }

		glm::vec3 GetForwardDirection() const {
			return glm::normalize(glm::vec3(-m_ViewMatrix[0][2], -m_ViewMatrix[1][2], -m_ViewMatrix[2][2]));
		}

		glm::vec3 GetRightDirection() const {
			return glm::normalize(glm::vec3(m_ViewMatrix[0][0], m_ViewMatrix[1][0], m_ViewMatrix[2][0]));
		}

		glm::vec3 GetUpDirection() const {
			return glm::normalize(glm::cross(GetRightDirection(), GetForwardDirection()));
		}

		float GetVerticalFOV() const { return m_PerspectiveFOV; }
		float GetAspectRatio() const { return m_AspectRatio; }

	private:
		void RecalculateProjection();
		void RecalculateView();
	private:
		ProjectionType m_ProjectionType = ProjectionType::Perspective;

		float m_PerspectiveFOV = glm::radians(45.0f);
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_AspectRatio = 0.0f;

		CameraMode m_Mode = CameraMode::ThirdPerson;

		glm::vec3 m_Target = glm::vec3(0.0f); // Entity to follow
		glm::vec3 m_Offset = glm::vec3(0.0f, 2.0f, 5.0f); // Third person offset
	};

}

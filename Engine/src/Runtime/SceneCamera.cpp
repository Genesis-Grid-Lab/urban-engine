#include "Runtime/SceneCamera.h"
#include "Core/UE_Assert.h"
#include "uepch.h"
#include <glm/gtc/matrix_transform.hpp>

namespace UE {

SceneCamera::SceneCamera() { RecalculateProjection(); }

void SceneCamera::SetPerspective(float verticalFOV, float nearClip,
                                 float farClip) {
  UE_PROFILE_FUNCTION();
  m_ProjectionType = ProjectionType::Perspective;
  m_PerspectiveFOV = verticalFOV;
  m_PerspectiveNear = nearClip;
  m_PerspectiveFar = farClip;
  RecalculateProjection();
}

void SceneCamera::SetOrthographic(float size, float nearClip, float farClip) {
  UE_PROFILE_FUNCTION();
  m_ProjectionType = ProjectionType::Orthographic;
  m_OrthographicSize = size;
  m_OrthographicNear = nearClip;
  m_OrthographicFar = farClip;
  RecalculateProjection();
}

void SceneCamera::SetViewportSize(uint32_t width, uint32_t height) {
  UE_PROFILE_FUNCTION();
  UE_CORE_ASSERT(width > 0 && height > 0);
  m_AspectRatio = (float)width / (float)height;
  RecalculateProjection();
}

void SceneCamera::RecalculateProjection() {
  UE_PROFILE_FUNCTION();
  if (m_ProjectionType == ProjectionType::Perspective) {
    m_ProjectionMatrix = glm::perspective(m_PerspectiveFOV, m_AspectRatio,
                                          m_PerspectiveNear, m_PerspectiveFar);
  } else {
    float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
    float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
    float orthoBottom = -m_OrthographicSize * 0.5f;
    float orthoTop = m_OrthographicSize * 0.5f;

    m_ProjectionMatrix =
        glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop,
                   m_OrthographicNear, m_OrthographicFar);
  }

  RecalculateView();
}

void SceneCamera::RecalculateView() {
  UE_PROFILE_FUNCTION();
  glm::vec3 direction;

  switch (m_Mode) {
  case CameraMode::ThirdPerson: {
    glm::vec3 behind = glm::normalize(m_Target - (m_Target + m_Offset));
    m_Position = m_Target + m_Offset;
    direction = glm::normalize(m_Target - m_Position);

    m_ViewMatrix =
        glm::lookAt(m_Position, m_Position + direction, glm::vec3(0, 1, 0));
    // m_Position = position;
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    break;
  }

  case CameraMode::FirstPerson: {
    m_Position = m_Target;
    direction = glm::normalize(
        glm::vec3(cos(m_Rotation2.y) * cos(m_Rotation2.x), sin(m_Rotation2.x),
                  sin(m_Rotation2.y) * cos(m_Rotation2.x)));

    m_ViewMatrix =
        glm::lookAt(m_Position, m_Position + direction, glm::vec3(0, 1, 0));
    // m_Position = position;
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    break;
  }

  case CameraMode::TopDown: {
    m_Position = m_Target + glm::vec3(0.0f, m_Offset.y, 0.0f);
    direction = glm::vec3(0.0f, -1.0f, 0.0f); // looking straight down

    m_ViewMatrix =
        glm::lookAt(m_Position, m_Position + direction, glm::vec3(0, 1, 0));
    // m_Position = position;
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    break;
  }

  case CameraMode::Mode2D: {
    m_Target = m_Position;
    auto Position = glm::vec3(m_Target.x, m_Target.y, 0.0f);

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), Position);

    // Optional rotation around Z if you want camera rotation
    // transform = glm::rotate(transform, m_RotationZ, glm::vec3(0,0,1));

    m_ViewMatrix = glm::inverse(transform);

    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    return;
  }
  }
}

} // namespace UE

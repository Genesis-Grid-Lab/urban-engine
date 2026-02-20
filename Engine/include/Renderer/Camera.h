#pragma once
#include "Core/Config.h"
#include "Core/Timestep.h"
#include <glm/glm.hpp>

namespace UE {
// Default camera values
const float Default_YAW = -90.0f;
const float Default_PITCH = 0.0f;
const float Default_SPEED = 5.5f;
const float Default_SENSITIVITY = 0.1f;
const float Default_ZOOM = 45.0f;

class Camera {
public:
  Camera() = default;
  Camera(const glm::mat4 &projection) : m_ProjectionMatrix(projection) {}
  virtual ~Camera() = default;
  void SetPosition(const glm::vec3 &position) {
    m_Position = position;
    RecalculateViewMatrix();
  }
  void SetRotation(float rotation) {
    m_Rotation = rotation;
    RecalculateViewMatrix();
  }
  void SetRotation2(const glm::vec3 &rotation) {
    m_Rotation2 = rotation;
    RecalculateViewMatrix();
  }

  float GetRotation() const { return m_Rotation; }
  const glm::vec3 &GetRotation2() const { return m_Rotation2; }
  const glm::vec3 &GetPosition() const { return m_Position; }
  const glm::mat4 &GetProjectionMatrix() const { return m_ProjectionMatrix; }
  const glm::mat4 &GetViewMatrix() const { return m_ViewMatrix; }
  const glm::mat4 &GetViewProjectionMatrix() const {
    return m_ViewProjectionMatrix;
  }

private:
  virtual void RecalculateViewMatrix() {
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), m_Position) *
        glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), {0, 0, 1});

    m_ViewMatrix = glm::inverse(transform);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
  };

protected:
  glm::vec3 m_Position;
  glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
  glm::mat4 m_ViewMatrix;
  glm::mat4 m_ViewProjectionMatrix;
  glm::vec3 m_Rotation2 = {0, 0, 0};
  float m_Rotation = 0.0f;
  float m_AspectRatio = 1.778f, m_FOV = 45.0f;
};

} // namespace UE

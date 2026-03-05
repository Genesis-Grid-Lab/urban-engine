#pragma once
#include "Entity.h"
#include "Scene.h"
#include "Timestep.h"

namespace UE {

class RuntimeScene : public Scene {
public:
  RuntimeScene() = default;
  RuntimeScene(uint32_t width, uint32_t height);
  virtual ~RuntimeScene() override;

  void OnRuntimeStart();
  void OnRuntimeStop();
  void PhysicsUpdate(float dt);
  virtual void OnUpdate(Timestep ts) override;

  void ClearColor(const glm::vec4 &color) { m_ClearColor = color; }

private:
  Camera &GetMainCamera();
  void FindPrimaryCamera();

  glm::vec4 m_ClearColor = {0.1f, 0.1f, 0.1f, 1};

  // Entity m_PrimaryCameraEntity;
  entt::entity m_PrimaryCameraEntity = entt::null;
};
} // namespace UE
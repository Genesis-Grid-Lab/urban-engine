#pragma once
#include "Auxiliaries/Physics.h"
#include "Components.h"
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

  private:
  Camera& GetMainCamera();
  void FindPrimaryCamera();

  PhysicsEngine m_Physics3D;

  // Entity m_PrimaryCameraEntity;
  entt::entity m_PrimaryCameraEntity = entt::null;
};
} // namespace UE
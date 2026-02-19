#pragma once
#include "Auxiliaries/Physics.h"
#include "Components.h"
#include "Config.h"
#include "Scene.h"
#include "Timestep.h"
#include "UUID.h"

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

  PhysicsEngine m_Physics3D;
};
} // namespace UE
#pragma once
#include <UrbanEngine.h>

using namespace UE;

#define now_width 1000
#define now_height 620

class PingPong : public Layer {
public:
  PingPong();

  virtual void OnAttach() override;
  virtual void OnUpdate(Timestep ts) override;
  virtual void OnEvent(Event &e) override;
  virtual void OnImGuiRender() override;

private:
  Ref<RuntimeScene> m_RuntimeScene;
  
  Entity Camera;
  Entity Player, Cpu, Ball;

  glm::vec4 yellow = {0.9529f, 0.8353f, 0.3569f, 1.0f};
  glm::vec4 Light_green = {0.5059f, 0.8000f, 0.7216f, 1.0f};
  glm::vec4 Green = {0.0314f, 0.7255f, 0.6039f, 1.0f};
  glm::vec4 colorDarGreen = {0.0784f, 0.6274f, 0.5215f, 1.0f};
};
#pragma once

#include "RuntimeScene.h"
#include "Timestep.h"
namespace UE {

class SceneManager {
public:
  SceneManager(uint32_t width, uint32_t height);

  void Update(Timestep ts);
  void Render();

  void Resize(uint32_t width, uint32_t height);

  void LoadScene(Ref<RuntimeScene> Scene);
  void CreateEmptyScene();

  Ref<RuntimeScene> GetActiveScene() {
    if (m_ActiveScene)
      return m_ActiveScene;

    return nullptr;
  }
  void StopActiveScene();

private:
  Ref<RuntimeScene> m_ActiveScene;

  uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;
};
} // namespace UE
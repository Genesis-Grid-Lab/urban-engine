#pragma once

#include <UrbanEngine.h>

using namespace UE;

class EditorScene : public Scene {
public:
  EditorScene(uint32_t width, uint32_t height);
  virtual ~EditorScene() override;

  virtual void OnUpdate(Timestep ts) override;

  const EditorCamera &GetCamera() { return m_EditorCamera; }

private:
  EditorCamera m_EditorCamera;
};
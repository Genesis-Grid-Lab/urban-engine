#include "PingPong.h"
#include "Components.h"
#include "Log.h"
#include "Runtime/RuntimeScene.h"

PingPong::PingPong() {}

void PingPong::OnAttach() {
  m_RuntimeScene = CreateRef<RuntimeScene>(now_width, now_height);

  Camera = m_RuntimeScene->CreateEntity("Camera");
  auto &CameraComp = Camera.AddComponent<CameraComponent>();
  auto &CameraTC = Camera.GetComponent<TransformComponent>();
  CameraComp.Primary = true;
  CameraComp.Camera.SetOrthographic(100, -1.0f, 1.0f);
  CameraComp.Camera.SetMode(CameraMode::Mode2D);
  CameraTC.Translation = {0.0f, 2.7f, 5.5f};

  auto circle = m_RuntimeScene->CreateEntity("Circle");
  auto circleComp = circle.AddComponent<CircleComponent>();
  circleComp.Color = Light_green;
  circleComp.Radius = 150;
}

void PingPong::OnUpdate(Timestep ts) {
  if (FramebufferSpecification spec =
          m_RuntimeScene->m_Framebuffer->GetSpecification();
      now_width > 0.0f &&
      now_height > 0.0f && // zero sized framebuffer is invalid
      (spec.Width != now_width || spec.Height != now_height)) {

    m_RuntimeScene->OnViewportResize((uint32_t)now_width, (uint32_t)now_height);
  }

  m_RuntimeScene->OnUpdate(ts);
  m_RuntimeScene->Draw();
}

void PingPong::OnEvent(Event &e) {}

void PingPong::OnImGuiRender() {

  ImGui::Begin("Viewport");

  ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
  glm::vec2 m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

  ImTextureID textureID =
      m_RuntimeScene->m_Framebuffer->GetColorAttachmentRendererID();
  ImGui::Image(textureID, ImVec2{m_ViewportSize.x, m_ViewportSize.y},
               ImVec2{0, 1}, ImVec2{1, 0});
  ImGui::End();
}
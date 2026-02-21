#include "PingPong.h"
#include "Components.h"
#include "Runtime/RuntimeScene.h"

static bool DrawVec3Control(const char *label, glm::vec3 &values,
                            float resetValue = 0.0f,
                            float columnWidth = 100.0f) {
  bool changed = false;

  ImGui::PushID(label);
  ImGui::PushID(&values); // <— unique per field instance

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, columnWidth);
  ImGui::TextUnformatted(label);
  ImGui::NextColumn();

  ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

  const float lineHeight =
      ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
  const ImVec2 btn = {lineHeight + 3.0f, lineHeight};

  auto axis = [&](const char *axisText, float &v, const ImVec4 &col,
                  const char *dragID) {
    ImGui::PushStyleColor(ImGuiCol_Button, col);
    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered,
        ImVec4{col.x + 0.1f, col.y + 0.1f, col.z + 0.1f, col.w});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, col);
    if (ImGui::Button(axisText, btn)) {
      v = resetValue;
      changed = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    changed |= ImGui::DragFloat(dragID, &v, 0.1f); // <— unique per axis
    ImGui::PopItemWidth();
    ImGui::SameLine();
  };

  axis("X", values.x, ImVec4{0.8f, 0.1f, 0.15f, 1.0f}, "##X");
  axis("Y", values.y, ImVec4{0.2f, 0.7f, 0.2f, 1.0f}, "##Y");
  axis("Z", values.z, ImVec4{0.1f, 0.25f, 0.8f, 1.0f}, "##Z");

  ImGui::PopStyleVar();
  ImGui::Columns(1);

  ImGui::PopID(); // &values
  ImGui::PopID(); // label
  return changed;
}

PingPong::PingPong() {}

void PingPong::OnAttach() {
  m_RuntimeScene = CreateRef<RuntimeScene>(now_width, now_height);

  Camera = m_RuntimeScene->CreateEntity("Camera");
  auto &CameraComp = Camera.AddComponent<CameraComponent>();
  auto &CameraTC = Camera.GetComponent<TransformComponent>();
  CameraComp.Primary = true;
  CameraComp.Camera.SetOrthographic(now_width, -1.0f, 1.0f);
  CameraComp.Camera.SetMode(CameraMode::Mode2D);
  CameraTC.Translation = {300.0f, 200.0f, 0.0f};

  auto circle = m_RuntimeScene->CreateEntity("Circle");
  auto &CircleC = circle.AddComponent<CircleComponent>();
  CircleC.Color = Light_green;
  CircleC.Radius = 150;
  auto &CircleTC = circle.GetComponent<TransformComponent>();
  CircleTC.Translation = {now_width / 2, now_height / 2, 0};

  auto backPlayer = m_RuntimeScene->CreateEntity("back");
  backPlayer.AddComponent<RectangleComponent>().Color = Green;
  auto &backTc = backPlayer.GetComponent<TransformComponent>();
  backTc.Translation = {now_width / 2 + (now_width / 2) / 2, now_height / 2, 0};
  backTc.Scale = {now_width / 2, now_height, 0};

  auto line = m_RuntimeScene->CreateEntity("line");
  auto &lineL = line.AddComponent<LineComponent>();
  lineL.Color = {1, 1, 1, 1};
  lineL.Thickness = 1;
  lineL.p0 = {now_width / 2, 0};
  lineL.p1 = {now_width / 2, now_height};

  Ball = m_RuntimeScene->CreateEntity("Ball");
  auto &ballC = Ball.AddComponent<CircleComponent>();
  ballC.Color = yellow;
  ballC.Radius = 20;
  // ballC.Order = 0;
  auto &ballTC = Ball.GetComponent<TransformComponent>();
  ballTC.Translation = {now_width / 2, now_height / 2, 0};

  Player = m_RuntimeScene->CreateEntity("Player");
  Player.AddComponent<RectangleComponent>().Color = {1, 1, 1, 1};
  auto &PlayerTC = Player.GetComponent<TransformComponent>();
  PlayerTC.Scale = {25, 120, 0};
  PlayerTC.Translation = {now_width - 25 - 10, now_height / 2, 0};

  Cpu = m_RuntimeScene->CreateEntity("Cpu");
  Cpu.AddComponent<RectangleComponent>().Color = {1, 1, 1, 1};
  auto &CpuTC = Cpu.GetComponent<TransformComponent>();
  CpuTC.Scale = {25, 120, 0};
  CpuTC.Translation = {10 + 25, now_height / 2, 0};

  m_RuntimeScene->OnRuntimeStart();
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

  auto &camTC = Camera.GetComponent<TransformComponent>();
  ImGui::Begin("monitor");

  DrawVec3Control("TranslationCam", camTC.Translation);

  ImGui::End();
}
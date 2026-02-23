#include "PingPong.h"
#include "Application.h"
#include "Components.h"
#include "Core/Input.h"
#include "Runtime/RuntimeScene.h"
#include "imgui.h"

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

bool CheckCircleRectCollision(const glm::vec2 &circlePos, float radius,
                              const glm::vec2 &rectCenter,
                              const glm::vec2 &rectSize) {
  glm::vec2 halfSize = rectSize * 0.5f;

  float closestX = glm::clamp(circlePos.x, rectCenter.x - halfSize.x,
                              rectCenter.x + halfSize.x);

  float closestY = glm::clamp(circlePos.y, rectCenter.y - halfSize.y,
                              rectCenter.y + halfSize.y);

  float dx = circlePos.x - closestX;
  float dy = circlePos.y - closestY;

  return (dx * dx + dy * dy) <= (radius * radius);
}

PingPong::PingPong() {}

void PingPong::OnAttach() {
  m_RuntimeScene = CreateRef<RuntimeScene>(now_width, now_height);
  m_RuntimeScene->ClearColor(colorDarGreen);

  Camera = m_RuntimeScene->CreateEntity("Camera");
  auto &CameraComp = Camera.AddComponent<CameraComponent>();
  auto &CameraTC = Camera.GetComponent<TransformComponent>();
  CameraComp.Primary = true;
  CameraComp.Camera.SetOrthographic(0, now_width, now_height, 0);
  CameraComp.Camera.SetMode(CameraMode::Mode2D);

  auto circle = m_RuntimeScene->CreateEntity("Circle");
  auto &CircleC = circle.AddComponent<CircleComponent>();
  CircleC.Color = Light_green;
  CircleC.Radius = 150;
  auto &CircleTC = circle.GetComponent<TransformComponent>();
  CircleTC.Translation = {now_width / 2, now_height / 2, -1};

  backPlayer = m_RuntimeScene->CreateEntity("back");
  backPlayer.AddComponent<RectangleComponent>().Color = Green;
  auto &backTc = backPlayer.GetComponent<TransformComponent>();
  backTc.Translation = {now_width / 2 + (now_width / 2) / 2, now_height / 2,
                        -2};
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

  ball_speed = {-10, 10};
  player_speed = 11;

  m_RuntimeScene->OnRuntimeStart();
}
void PingPong::BallLogic() {
  auto &ballTC = Ball.GetComponent<TransformComponent>().Translation;
  auto &ballC = Ball.GetComponent<CircleComponent>();

  ballTC.x += ball_speed.x;
  ballTC.y += ball_speed.y;

  float x = ballTC.x, y = ballTC.y, radius = ballC.Radius;

  if (y + radius >= now_height || y - radius <= 0) {
    ball_speed.y *= -1;
  }

  if (x + radius >= now_width) {
    Cpu_score++;
    ballTC.x = (float)now_width / 2;
    ballTC.y = (float)now_height / 2;

    int speed_choices[2] = {-1, 1};
    // ball_speed.x *= speed_choices[GetRandomValue(0, 1)];
    // ball_speed.y *= speed_choices[GetRandomValue(0, 1)];
  }

  if (x - radius <= 0) {
    Player_score++;
    ballTC.x = (float)now_width / 2;
    ballTC.y = (float)now_height / 2;

    int speed_choices[2] = {-1, 1};
    // ball_speed.x *= speed_choices[GetRandomValue(0, 1)];
    // ball_speed.y *= speed_choices[GetRandomValue(0, 1)];
  }
}

void PingPong::PlayerLogic() {
  auto &playerTC = Player.GetComponent<TransformComponent>().Translation;

  if (Input::IsKeyPressed(Key::Up)) {
    playerTC.y -= player_speed;
  }

  if (Input::IsKeyPressed(Key::Down)) {
    playerTC.y += player_speed;
  }

  // limit movement
  int height = 60;

  if (playerTC.y - height <= 0) {
    playerTC.y = height;
  }
  if (playerTC.y + height >= now_height) {
    playerTC.y = now_height - height;
  }
}

void PingPong::CpuLogic() {
  auto &cpuTC = Cpu.GetComponent<TransformComponent>().Translation;
  auto &ballTC = Ball.GetComponent<TransformComponent>().Translation;

  float speed = 8.0f;

  if (ballTC.y > cpuTC.y + 10)
    cpuTC.y += speed;
  else if (ballTC.y < cpuTC.y - 10)
    cpuTC.y -= speed;

  // limit movement
  int height = 60;

  if (cpuTC.y - height <= 0)
    cpuTC.y = height;

  if (cpuTC.y + height >= now_height)
    cpuTC.y = now_height - height;
}

void PingPong::GameLogic() {

  auto &ballTC = Ball.GetComponent<TransformComponent>().Translation;
  auto &ballC = Ball.GetComponent<CircleComponent>();

  auto &playerTC = Player.GetComponent<TransformComponent>();
  auto &cpuTC = Cpu.GetComponent<TransformComponent>();

  // Player paddle collision
  if (CheckCircleRectCollision({ballTC.x, ballTC.y}, ballC.Radius,
                               {playerTC.Translation.x, playerTC.Translation.y},
                               {playerTC.Scale.x, playerTC.Scale.y})) {
    ball_speed.x *= -1;
  }

  // CPU paddle collision
  if (CheckCircleRectCollision({ballTC.x, ballTC.y}, ballC.Radius,
                               {cpuTC.Translation.x, cpuTC.Translation.y},
                               {cpuTC.Scale.x, cpuTC.Scale.y})) {
    ball_speed.x *= -1;
  }
}

void PingPong::OnUpdate(Timestep ts) {
  if (FramebufferSpecification spec =
          m_RuntimeScene->m_Framebuffer->GetSpecification();
      now_width > 0.0f &&
      now_height > 0.0f && // zero sized framebuffer is invalid
      (spec.Width != now_width || spec.Height != now_height)) {

    Application::Get().GetSceneManager().Resize(now_width, now_height);
  }

  if (Game_started) {
    BallLogic();
    PlayerLogic();
    CpuLogic();
    GameLogic();
  }

  RuntimeStart e;
  if (RuntimeEventBus::Poll(e)) {
    Application::Get().GetSceneManager().LoadScene(m_RuntimeScene);
    UE_WARN("Start");
    Game_started = true;
  }

  Application::Get().GetSceneManager().Update(ts);
  Application::Get().GetSceneManager().Render();
}

void PingPong::OnEvent(Event &e) {}

void PingPong::OnImGuiRender() {

  auto &camTC = Camera.GetComponent<TransformComponent>();
  auto &playerTC = Player.GetComponent<TransformComponent>();
  auto &backTC = backPlayer.GetComponent<TransformComponent>();
  ImGui::Begin("monitor");

  DrawVec3Control("TranslationCam", camTC.Translation);
  DrawVec3Control("Player", playerTC.Translation);
  DrawVec3Control("BackPlayer", backTC.Translation);

  if (ImGui::Button("Play")) {
    RuntimeEventBus::Emit<RuntimeStart>();
  }

  ImGui::Text("player score: %i", Player_score);
  ImGui::Text("cpu score: %i", Cpu_score);

  ImGui::End();
}
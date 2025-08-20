#pragma once

#include "EditorLayer.h"

class PlayerController : public ScriptableEntity
{
 public:
  virtual void OnCreate() override
  {
    UE_INFO("OnCreate");
    m_Scene = GetScene();
    // Randomize X a little, keep Y/Z as authored
    auto &tc = GetComponent<TransformComponent>();
    tc.Translation.x = (float)((rand() % 10) - 5);

    // Find primary camera once
    auto view = m_Scene->GetRegistry().view<TransformComponent, CameraComponent>();
    for (auto entity : view) {
      auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
      if (camera.Primary) {
	UE_INFO("CAM FOUND");
	Cam = Entity(entity, m_Scene);
	break;
      }
    }
    // m_Scene->ViewEntity<Entity, CameraComponent>([&](auto e, auto &CamComp) {
    //   UE_INFO("Searching cam");
    //   if (CamComp.Primary) {
    // 	UE_INFO("CAM FOUND");
    //     Cam = Entity(e, m_Scene);
    // 	// break;
    //   }
    // });
  }

  virtual void OnDestroy() override
  {
  }

  virtual void OnUpdate(Timestep ts) override
  {
    // Components we’ll touch
        auto &charComp  = GetComponent<CharacterComponent>();         // NEW API: provides WishMove, MaxSpeed, Body, etc.
        auto &tc        = GetComponent<TransformComponent>();
        auto &camTC     = Cam.GetComponent<TransformComponent>();
        auto &camData   = Cam.GetComponent<CameraComponent>().Camera;

        // Optional: animation data (if present)
        auto &model     = GetComponent<ModelComponent>().ModelData;
        auto &modelAnim = GetComponent<ModelComponent>().AnimationData;

        const float dt = (float)ts;
        const bool  rmb = Input::IsMouseButtonPressed(Mouse::ButtonRight);

        // --- Orbit camera with RMB drag ---
        if (rmb) {
            const glm::vec2 md = Input::GetMouseDelta();
            // Avoid fmt issues with glm types: log scalars if needed
            // UE_CORE_INFO("mouse dx={}, dy={}", md.x, md.y);
            m_Yaw   += md.x * m_MouseSensitivity;
            m_Pitch += md.y * m_MouseSensitivity;
            m_Pitch  = glm::clamp(m_Pitch, -89.0f, 89.0f);
        }

        // Camera direction from yaw/pitch
        glm::vec3 camDir;
        camDir.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        camDir.y = sin(glm::radians(m_Pitch));
        camDir.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        camDir   = glm::normalize(camDir);

        // Camera target just above player head
        const glm::vec3 camTarget = tc.Translation + glm::vec3(0, 1, 0);
        const glm::vec3 camPos    = camTarget - camDir * m_Distance;
        camTC.Translation = camPos;

        // Camera follow params
        camData.SetTarget(camTarget);
        camData.SetMode(m_Mode);
        camData.SetOffset(camDir * 5.0f + glm::vec3(0, 2, 0)); // third-person offset

        // --- Movement input (relative to camera heading) ---
        glm::vec3 forward = glm::normalize(glm::vec3(camDir.x, 0.0f, camDir.z));
        glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

        glm::vec3 moveDir(0.0f);
        if (Input::IsKeyPressed(Key::W)) moveDir -= forward;
        if (Input::IsKeyPressed(Key::S)) moveDir += forward;
        if (Input::IsKeyPressed(Key::A)) moveDir += right;
        if (Input::IsKeyPressed(Key::D)) moveDir -= right;

        const bool wantMove = glm::length2(moveDir) > 0.0f;
        if (wantMove) moveDir = glm::normalize(moveDir);

        // Sprint (optional)
        float speed = charComp.MaxSpeed;
        if (Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift))
            speed *= 1.6f;

        // NEW API: write the desired horizontal velocity; physics system consumes this in its MoveKinematic step
        charComp.WishMove = moveDir * speed;

        // Jump flag (if you later add vertical handling in your PhysicsUpdate for characters)
        charComp.Jump = Input::IsKeyPressed(Key::Space);

        // Face movement direction if moving
        if (wantMove) {
            // atan2(x, z) → yaw radians
            const float targetYawDeg = glm::degrees(atan2(moveDir.x, moveDir.z));
            // simple snap; replace with smoothing if you like
            tc.Rotation.y = glm::radians(targetYawDeg);
        }

        // --- Basic animation state ---
        bool running = wantMove && !charComp.Jump;
        bool idle    = !wantMove && !charComp.Jump;
        bool jumping = charComp.Jump;

	auto playAnimIf = [&](const char* key) {
	  auto it = modelAnim.find(key);
	  if (it != modelAnim.end() && it->second) {
	    Renderer3D::RunAnimation(it->second, ts);
	    return true;
	  }
	  return false;
	};

	if (running)      playAnimIf("run");
	else if (jumping) playAnimIf("jump");
	else              playAnimIf("idle");


        /* if (running && modelAnim.contains("run")) */
        /*     Renderer3D::RunAnimation(modelAnim["run"], ts); */
        /* else if (jumping && modelAnim.contains("jump")) */
        /*     Renderer3D::RunAnimation(modelAnim["jump"], ts); */
        /* else if (idle && modelAnim.contains("idle")) */
        /*     Renderer3D::RunAnimation(modelAnim["idle"], ts); */
	UE_INFO("PC WishMove len={:.3f} jump={}", glm::length(charComp.WishMove), (int)charComp.Jump);
    }

    void OnImGuiRender() override {
        ImGui::Text("Player Controller");
        ImGui::SliderFloat("Camera Distance", &m_Distance, 2.0f, 12.0f);
        ImGui::SliderFloat("Mouse Sensitivity", &m_MouseSensitivity, 0.02f, 0.6f);
    }

 private:
  Scene* m_Scene;
  Entity Cam;
  CameraMode m_Mode = CameraMode::ThirdPerson;
  float m_Yaw = 0.0f;
  float m_Pitch = 0.0f;
  float m_MouseSensitivity = 0.1f;
  float m_Distance = 5.0f; // Camera distance behind player
};

#include "Runtime/RuntimeScene.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Renderer3D.h"
#include "Runtime/Components.h"
#include "Runtime/Entity.h"
#include "Runtime/ScriptableEntity.h"

namespace UE {

RuntimeScene::RuntimeScene() {}
RuntimeScene::~RuntimeScene() {}

// ------------------------------
// Scene::OnRuntimeStart
// ------------------------------

void RuntimeScene::OnRuntimeStart() {

  ViewEntity<Entity, NativeScriptComponent>([=](auto entity, auto &nsc) {
    // JPH::BodyInterface &body_interface = m_Physics3D.Bodies();

    if (!nsc.Instance) {
      UE_CORE_ASSERT(nsc.InstantiateScript,
                     "NativeScriptComponent missing Bind()");
      nsc.Instance = nsc.InstantiateScript();
      // >>> set context first <<<
      nsc.Instance->m_Entity = Entity{entity, this};
      nsc.Instance->m_Scene = this;
      // nsc.Instance->m_BodyInterface = &body_interface;
      // now it's safe to enter user code
      nsc.Instance->OnCreate();
    } else {
      // keep context up to date for already-instantiated scripts
      nsc.Instance->m_Entity = Entity{entity, this};
      nsc.Instance->m_Scene = this;
      // nsc.Instance->m_BodyInterface = &body_interface;
    }
  });
}

// ------------------------------
// Scene::OnRuntimeStop
// ------------------------------

void RuntimeScene::OnRuntimeStop() { UE_PROFILE_FUNCTION(); }

// ------------------------------
// Scene::PhysicsUpdate
// ------------------------------

void RuntimeScene::PhysicsUpdate(float dt) { UE_PROFILE_FUNCTION(); }

void RuntimeScene::OnUpdate(Timestep ts) {
  UE_PROFILE_FUNCTION();
  m_Framebuffer->Bind();
  // Clear our entity ID attachment to -1
  m_Framebuffer->ClearAttachment(1, -1);
  RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
  RenderCommand::Clear();

  // Update scripts
  {
    m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto &nsc) {
      // JPH::BodyInterface &body_interface = m_Physics3D.Bodies();
      // TODO: Move to Scene::OnScenePlay
      if (!nsc.Instance) {
        UE_CORE_ASSERT(nsc.InstantiateScript,
                       "NativeScriptComponent missing Bind()");
        nsc.Instance = nsc.InstantiateScript();
        // >>> set context first <<<
        nsc.Instance->m_Entity = Entity{entity, this};
        nsc.Instance->m_Scene = this;
        // nsc.Instance->m_BodyInterface = &body_interface;
        // now it's safe to enter user code
        nsc.Instance->OnCreate();
      } else {
        // keep context up to date for already-instantiated scripts
        nsc.Instance->m_Entity = Entity{entity, this};
        nsc.Instance->m_Scene = this;
        // nsc.Instance->m_BodyInterface = &body_interface;
      }

      nsc.Instance->OnUpdate(ts);
    });
  }

  PhysicsUpdate(ts);

  // Render 3D
  Camera *mainCamera = nullptr;
  glm::mat4 cameraTransform;
  glm::vec3 pos;
  TransformComponent tc;
  {

    ViewEntity<Entity, CameraComponent>(
        [this, &mainCamera, &pos, &tc](auto entity, auto &comp) {
          auto &transform = entity.template GetComponent<TransformComponent>();
          comp.Camera.m_Position = &transform.Translation;
          // comp.Camera.m_Rotation2 = &transform.Rotation;
          if (comp.Primary) {
            mainCamera = &comp.Camera;
            pos = transform.Translation;
            tc = transform;
          }
        });
  }

  if (mainCamera) {

    // Renderer3D::BeginCamera(*mainCamera, tc);
    Renderer3D::BeginCamera(*mainCamera);

    GroupEntity<LightComponent>(
        [this](auto entity, auto &comp, auto &transform, auto id) {
          Renderer3D::RenderLight(transform.Translation, comp.Color);
        });

    GroupEntity<ModelComponent>(
        [this](auto entity, auto &comp, auto &transform, auto id) {
          Renderer3D::DrawModel(comp.ModelData, transform.GetTransform(),
                                glm::vec3(1.0f), (int)id);
        });

    auto CubeGroup =
        m_Registry.group<CubeComponent>(entt::get<TransformComponent>);
    for (auto entity : CubeGroup) {
      auto [transform, CubeComp] =
          CubeGroup.get<TransformComponent, CubeComponent>(entity);

      Renderer3D::DrawCube(transform.GetTransform(), CubeComp.Color,
                           (int)entity);
    }

    Renderer3D::EndCamera();

    Renderer2D::BeginCamera(*mainCamera);
    ViewEntity<Entity, UIElement>([this](auto entity, auto &comp) {
      auto &transform = entity.template GetComponent<TransformComponent>();
      // Renderer2D::DrawUI(transform.GetTransform(), comp);
    });

    auto group1 =
        m_Registry.group<ButtonComponent>(entt::get<TransformComponent>);
    for (auto entity : group1) {

      auto [transform, ui] =
          group1.get<TransformComponent, ButtonComponent>(entity);
      // Renderer2D::DrawUI(transform.GetTransform(), ui, (int)entity);
    }

    ViewEntity<Entity, TextUIComponent>([this](auto entity, auto &comp) {
      auto &transform = entity.template GetComponent<TransformComponent>();
      // Renderer2D::DrawUI(transform.Translation, comp);
    });

    auto Spritegroup = m_Registry.group<SpriteRendererComponent>(
        entt::get<TransformComponent>);
    for (auto entity : Spritegroup) {
      auto [transform, sprite] =
          Spritegroup.get<TransformComponent, SpriteRendererComponent>(entity);
      // Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
    }

    // ViewEntity<Entity, SpriteRendererComponent>([this] (auto entity, auto&
    // comp){

    // 	auto& transform = entity.template GetComponent<TransformComponent>();
    // 	Renderer2D::DrawSprite(transform.GetTransform(), comp, (int)entity);
    // });

    Renderer2D::EndCamera();
  }

  // ReadPixelEntity(mouseX, mouseY, viewportSize);

  m_Framebuffer->Unbind();

  RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
  RenderCommand::Clear();

  FlushEntityDestruction();
}
} // namespace UE
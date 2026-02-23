#include "Runtime/RuntimeScene.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Renderer3D.h"
#include "Runtime/Components.h"
#include "Runtime/Entity.h"
#include "Runtime/ScriptableEntity.h"

namespace UE {

RuntimeScene::RuntimeScene(uint32_t width, uint32_t height) {
  UE_PROFILE_FUNCTION();
  m_ViewportWidth = width;
  m_ViewportHeight = height;

  FramebufferSpecification fbSpec;
  fbSpec.Attachments = {FramebufferTextureFormat::RGBA8,
                        FramebufferTextureFormat::RED_INTEGER,
                        FramebufferTextureFormat::Depth};
  fbSpec.Width = width;
  fbSpec.Height = height;
  m_Framebuffer = Framebuffer::Create(fbSpec);
}
RuntimeScene::~RuntimeScene() {}

// ------------------------------
// Scene::OnRuntimeStart
// ------------------------------

void RuntimeScene::OnRuntimeStart() {

  size_t count = m_Registry.alive();
  UE_CORE_INFO("Alive entities: {}", count);

  FindPrimaryCamera();

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

void RuntimeScene::FindPrimaryCamera() {
  auto view = m_Registry.view<CameraComponent>();
  for (auto entity : view) {
    if (view.get<CameraComponent>(entity).Primary) {
      m_PrimaryCameraEntity = entity;
      return;
    }
  }

  m_PrimaryCameraEntity = entt::null;
}

Camera &RuntimeScene::GetMainCamera() {

  UE_CORE_ASSERT(m_PrimaryCameraEntity != entt::null, "No Primary Camera!");

  auto &camComp = m_Registry.get<CameraComponent>(m_PrimaryCameraEntity);
  auto &transform = m_Registry.get<TransformComponent>(m_PrimaryCameraEntity);

  camComp.Camera.SetPosition(transform.Translation);

  return camComp.Camera;
}

void RuntimeScene::OnUpdate(Timestep ts) {
  UE_PROFILE_FUNCTION();
  m_Framebuffer->Bind();
  // Clear our entity ID attachment to -1
  m_Framebuffer->ClearAttachment(1, -1);
  RenderCommand::SetClearColor(m_ClearColor);
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

  Renderer3D::BeginCamera(GetMainCamera());

  GroupEntity<SkyboxComponent>(
      [this](auto entity, auto &comp, auto &transform, auto id) {
        Renderer3D::DrawSkybox(comp.skybox, *comp.Cam);
      });

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

    Renderer3D::DrawCube(transform.GetTransform(), CubeComp.Color, (int)entity);
  }

  Renderer3D::EndCamera();

  Renderer2D::BeginCamera(GetMainCamera());
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

  auto Spritegroup =
      m_Registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);
  for (auto entity : Spritegroup) {
    auto [transform, sprite] =
        Spritegroup.get<TransformComponent, SpriteRendererComponent>(entity);
    // Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
  }

  GroupEntity<CircleComponent>([this](auto entity, auto &comp, auto &transform,
                                      auto id) {
    UE_CORE_TRACE("entity: {}, id: {}", (uint32_t)entity, (uint32_t)id);
    Renderer2D::DrawCircle(transform.Translation,
                           comp.Radius, comp.Color, entity, 1);
  });

  GroupEntity<RectangleComponent>(
      [this](auto entity, auto &comp, auto &transform, auto id) {
        Renderer2D::DrawQuad(transform.GetTransform(), comp.Color, entity);
      });

  GroupEntity<LineComponent>([this](auto entity, auto &comp, auto &transform,
                                    auto id) {
    Renderer2D::DrawLine(comp.p0, comp.p1, comp.Thickness, comp.Color, entity);
  });

  // ViewEntity<Entity, SpriteRendererComponent>([this] (auto entity, auto&
  // comp){

  // 	auto& transform = entity.template GetComponent<TransformComponent>();
  // 	Renderer2D::DrawSprite(transform.GetTransform(), comp, (int)entity);
  // });

  // Renderer2D::DrawQuad({0, 0}, {10, 10}, {0, 1, 0, 1});

  Renderer2D::EndCamera();

  // ReadPixelEntity(mouseX, mouseY, viewportSize);

  m_Framebuffer->Unbind();

  RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
  RenderCommand::Clear();

  FlushEntityDestruction();
}
} // namespace UE
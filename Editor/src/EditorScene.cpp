#include "EditorScene.h"

EditorScene::EditorScene(uint32_t width, uint32_t height) {
  m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 2000.0f);
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
EditorScene::~EditorScene() {}

void EditorScene::OnUpdate(Timestep ts) {
  m_EditorCamera.OnUpdate(ts);
  m_EditorCamera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
  UE_PROFILE_FUNCTION();
  m_Framebuffer->Bind();
  // Clear our entity ID attachment to -1
  m_Framebuffer->ClearAttachment(1, -1);
  RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
  RenderCommand::Clear();

  // m_Physics3D.Simulate(ts);
  // glEnable(GL_DEPTH_TEST);
  Renderer3D::BeginCamera(m_EditorCamera);

  GroupEntity<LightComponent>(
      [this](auto entity, auto &comp, auto &transform, auto id) {
        Renderer3D::RenderLight(transform.Translation, comp.Color);
      });

  GroupEntity<ModelComponent>(
      [this](auto entity, auto &comp, auto &transform, auto id) {
        Renderer3D::DrawModel(comp.ModelData, transform.GetTransform(),
                              glm::vec3(1.0f), 1.0f, (int)id);
      });

  GroupEntity<CubeComponent>([this](auto entity, auto &comp, auto &transform,
                                    auto id) {
    Renderer3D::DrawCube(transform.GetTransform(), comp.Color, 1.0f, (int)id);
  });

  GroupEntity<CameraComponent>([this](auto entity, auto &comp, auto &transform,
                                      auto id) {
    comp.Camera.m_Position = &transform.Translation;
    // CamComp.Camera.m_Rotation2 = &transform.Rotation;
    if (ShowCams)
      Renderer3D::DrawCube(transform.GetTransform(), {1, 0, 0}, 1.0f, (int)id);
    // Renderer3D::DrawCameraFrustum(comp.Camera);

    // UE_CORE_WARN("Camera entity: {}", (int)id);
  });

  Renderer3D::EndCamera();
  // glDisable(GL_DEPTH_TEST);
  Renderer2D::BeginCamera(m_EditorCamera);
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

  // ViewEntity<Entity, SpriteRendererComponent>([this] (auto entity, auto&
  // comp){

  // 	auto& transform = entity.template GetComponent<TransformComponent>();
  // 	Renderer2D::DrawSprite(transform.GetTransform(), comp, (int)entity);
  // });

  Renderer2D::EndCamera();

  // ReadPixelEntity(mouseX, mouseY, viewportSize);

  m_Framebuffer->Unbind();

  RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
  RenderCommand::Clear();

  FlushEntityDestruction();
}
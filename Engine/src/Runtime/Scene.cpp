#include "Runtime/Scene.h"
#include "Core/Log.h"
#include "Runtime/Components.h"
#include "Runtime/Entity.h"
#include "Runtime/RuntimeScene.h"
#include "UE_Assert.h"
#include "entity/fwd.hpp"
#include "uepch.h"

namespace UE {

Entity GlobHovered;

template <typename Component>
static void
CopyComponent(entt::registry &dst, entt::registry &src,
              const std::unordered_map<UUID, entt::entity> &enttMap) {
  auto view = src.view<Component>();
  for (auto e : view) {
    UUID uuid = src.get<IDComponent>(e).ID;
    UE_CORE_ASSERT(enttMap.find(uuid) != enttMap.end());
    entt::entity dstEnttID = enttMap.at(uuid);

    auto &component = src.get<Component>(e);
    dst.emplace_or_replace<Component>(dstEnttID, component);
  }
}

template <typename Component>
static void CopyComponentIfExists(Entity dst, Entity src) {
  if (src.HasComponent<Component>())
    dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
}

Ref<RuntimeScene> Scene::Copy(const Ref<Scene> &other) {
  UE_CORE_INFO("[COPY]: scene={}", (const void *)&other);

  Ref<RuntimeScene> newScene = CreateRef<RuntimeScene>();

  //   newScene->ShowBoxes = other->ShowBoxes;
  //   newScene->ShowBoxesPlay = other->ShowBoxesPlay;
  //   newScene->ShowCams = other->ShowCams;
  // newScene->m_Physics3D. = other->m_Physics3D;
  // auto& srcSceneRegistry = other->m_Registry;
  auto &srcSceneRegistry = other->m_Registry;
  auto &dstSceneRegistry = newScene->m_Registry;
  std::unordered_map<UUID, entt::entity> enttMap;
  newScene->m_Framebuffer = other->m_Framebuffer;

  // Create entities in new scene
  auto idView = srcSceneRegistry.view<IDComponent>();
  for (auto e : idView) {
    UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
    const auto &name = srcSceneRegistry.get<TagComponent>(e).Tag;
    Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
    enttMap[uuid] = (entt::entity)newEntity;
    auto &srcT = srcSceneRegistry.get<TransformComponent>(e);
    auto &dstT =
        dstSceneRegistry.get<TransformComponent>((entt::entity)newEntity);
  }

  // Copy components (except IDComponent and TagComponent)
  CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry,
                                    enttMap);
  CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry,
                                         enttMap);
  CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
  CopyComponent<ModelComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
  CopyComponent<CubeComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
  CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry,
                                       enttMap);
  CopyComponent<CircleComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
  CopyComponent<RectangleComponent>(dstSceneRegistry, srcSceneRegistry,
                                    enttMap);
  CopyComponent<LineComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
  CopyComponent<LightComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
  CopyComponent<SkyboxComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
  CopyComponent<RigidbodyComponent>(dstSceneRegistry, srcSceneRegistry,
                                    enttMap);
  CopyComponent<BoxColliderComponent>(dstSceneRegistry, srcSceneRegistry,
                                      enttMap);

  return newScene;
  // return nullptr;
}

Entity Scene::CreateEntity(const std::string &name) {
  return CreateEntityWithUUID(UUID(), name);
}

Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name) {
  Entity entity = {m_Registry.create(), this};
  entity.AddComponent<IDComponent>(uuid);
  entity.AddComponent<TransformComponent>();
  auto &tag = entity.AddComponent<TagComponent>();
  tag.Tag = name.empty() ? "Entity" : name;
  return entity;
}

void Scene::DestroyEntityNow(Entity entity) { m_Registry.destroy(entity); }

void Scene::DestroyEntity(Entity entity) {
  // Queue for destruction; actual registry destroy happens in
  m_DestroyQueue.push_back((entt::entity)entity);
}

void Scene::ReadPixelEntity(int &mouseX, int &mouseY, glm::vec2 &viewportSize) {
  if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x &&
      mouseY < (int)viewportSize.y) {
    int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
    // UE_INFO("mx: {0}, my: {1}", Input::GetMouseX(), Input::GetMouseY());
    if (pixelData < -1 || pixelData >= (int)99036831949)
      pixelData = -1;
    // UE_INFO("Pixel {0}", pixelData);
    GlobHovered =
        pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, this);
  }
}

void Scene::OnViewportResize(uint32_t width, uint32_t height) {
  UE_PROFILE_FUNCTION();
  m_ViewportWidth = width;
  m_ViewportHeight = height;

  m_Framebuffer->Resize(width, height);

  // Resize our non-FixedAspectRatio cameras
  auto view = m_Registry.view<CameraComponent>();
  for (auto entity : view) {
    auto &cameraComponent = view.get<CameraComponent>(entity);
    if (!cameraComponent.FixedAspectRatio)
      cameraComponent.Camera.SetViewportSize(width, height);
  }
}

void Scene::OnMouseInput(float mouseX, float mouseY, bool mousePressed,
                         Timestep ts) {
  auto group1 =
      m_Registry.group<ButtonComponent>(entt::get<TransformComponent>);
  for (auto entity : group1) {

    auto [transform, button] =
        group1.get<TransformComponent, ButtonComponent>(entity);

    float halfWidth = transform.Scale.x * 0.5f;
    float halfHeight = transform.Scale.y * 0.5f;

    bool hovered = mouseX >= (transform.Translation.x - halfWidth) &&
                   mouseX <= (transform.Translation.x + halfWidth) &&
                   mouseY >= (transform.Translation.y - halfHeight) &&
                   mouseY <= (transform.Translation.y + halfHeight);

    button.Hovered = hovered;

    if (hovered && mousePressed && !button.ClickedLastFrame) {
      if (button.OnClick)
        button.OnClick();
      button.ClickedLastFrame = true;
    } else if (!mousePressed) {
      button.ClickedLastFrame = false;
    }

    // Animate scale
    if (button.ClickedLastFrame)
      button.TargetScale = button.OriginalScale * 0.95f;
    else if (button.Hovered)
      button.TargetScale = button.OriginalScale * 1.05f;
    else
      button.TargetScale = button.OriginalScale;

    transform.Scale = glm::mix(transform.Scale, button.TargetScale, 0.2f);

    button.BaseColor = button.Color;
    button.CurrentColor = button.Color;

    // Animate Color
    glm::vec4 hoverColor = button.BaseColor * 1.2f;
    hoverColor.a = button.BaseColor.a; // preserve alpha

    button.CurrentColor =
        glm::mix(button.CurrentColor,
                 button.Hovered ? hoverColor : button.BaseColor, 0.2f);
    button.Color = button.CurrentColor;
  }

  m_MouseX = mouseX;
  m_MouseY = mouseY;
}

Entity Scene::GetHoveredEntity() {

  if (GlobHovered)
    return GlobHovered;
  else
    return Entity();
}

void Scene::FlushEntityDestruction() {
  if (m_DestroyQueue.empty())
    return;
  for (auto e : m_DestroyQueue)
    if (m_Registry.valid(e))
      m_Registry.destroy(e);

  m_DestroyQueue.clear();
}

void Scene::DuplicateEntity(Entity entity) {
  std::string name = entity.GetName();
  Entity newEntity = CreateEntity(name);

  CopyComponentIfExists<TransformComponent>(newEntity, entity);
  CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
  CopyComponentIfExists<CameraComponent>(newEntity, entity);
}

template <typename T>
void Scene::OnComponentAdded(Entity entity, T &component) {
  // static_assert(false);
}

template <>
void Scene::OnComponentAdded<IDComponent>(Entity entity,
                                          IDComponent &component) {}

template <>
void Scene::OnComponentAdded<TransformComponent>(
    Entity entity, TransformComponent &component) {}

template <>
void Scene::OnComponentAdded<SpriteRendererComponent>(
    Entity entity, SpriteRendererComponent &component) {}

template <>
void Scene::OnComponentAdded<UIElement>(Entity entity, UIElement &component) {}

template <>
void Scene::OnComponentAdded<ButtonComponent>(Entity entity,
                                              ButtonComponent &component) {}

template <>
void Scene::OnComponentAdded<TextUIComponent>(Entity entity,
                                              TextUIComponent &component) {}

template <>
void Scene::OnComponentAdded<TagComponent>(Entity entity,
                                           TagComponent &component) {}

template <>
void Scene::OnComponentAdded<ModelComponent>(Entity entity,
                                             ModelComponent &component) {}

template <>
void Scene::OnComponentAdded<CubeComponent>(Entity entity,
                                            CubeComponent &component) {}

template <>
void Scene::OnComponentAdded<LightComponent>(Entity entity,
                                             LightComponent &component) {}

template <>
void Scene::OnComponentAdded<NativeScriptComponent>(
    Entity entity, NativeScriptComponent &component) {}

template <>
void Scene::OnComponentAdded<CameraComponent>(Entity entity,
                                              CameraComponent &component) {
  if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
    component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
}

template <>
void Scene::OnComponentAdded<SkyboxComponent>(Entity entity,
                                              SkyboxComponent &component) {}

template <>
void Scene::OnComponentAdded<CircleComponent>(Entity entity,
                                              CircleComponent &component) {}

template <>
void Scene::OnComponentAdded<RectangleComponent>(
    Entity entity, RectangleComponent &component) {}

template <>
void Scene::OnComponentAdded<LineComponent>(Entity entity,
                                            LineComponent &component) {}

template <>
void Scene::OnComponentAdded<AnimatorComponent>(Entity entity,
                                                AnimatorComponent &component) {}

template <>
void Scene::OnComponentAdded<BoxColliderComponent>(
    Entity entity, BoxColliderComponent &component) {}

template <>
void Scene::OnComponentAdded<RigidbodyComponent>(
    Entity entity, RigidbodyComponent &component) {}
} // namespace UE

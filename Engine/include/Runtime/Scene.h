#pragma once

#include "Auxiliaries/Physics.h"
#include "Components.h"
#include "Config.h"
#include "Renderer/Camera.h"
#include "Renderer/EditorCamera.h"
#include "Renderer/Model.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Timestep.h"
#include "UE_Assert.h"
#include "UUID.h"
#include <entt.hpp>

namespace UE {

class Entity;
class RuntimeScene;

class Scene {
public:
  // Scene(uint32_t width, uint32_t height);
  virtual ~Scene() = default;

  static Ref<RuntimeScene> Copy(const Ref<Scene> &other);

  Entity CreateEntity(const std::string &name = std::string());
  Entity CreateEntityWithUUID(UUID uuid,
                              const std::string &name = std::string());
  void DestroyEntity(Entity entity);
  void DestroyEntityNow(Entity entity); // immediate (use with care)
  void FlushEntityDestruction();        // call at end of update/frame

  void Draw() { m_Framebuffer->DrawBuffer(m_ViewportWidth, m_ViewportHeight); }

  template <typename Entt, typename Comp, typename Task>
  void ViewEntity(Task &&task) {
    // UE_CORE_ASSERT(std::is_base_of<Entity, Entt>::value, "error viewing
    // entt");
    m_Registry.view<Comp>().each([this, &task](const auto entity, auto &comp) {
      // task(std::move(Entt(&m_Registry, entity)), comp);
      task(std::move(Entt(entity, this)), comp);
    });
  }

  template <typename Comp, typename Task> void GroupEntity(Task &&task) {
    auto group = m_Registry.group<Comp>(entt::get<TransformComponent>);
    for (auto entity : group) {
      const auto &[transform, comp] =
          group.template get<TransformComponent, Comp>(entity);
      task(std::move(Entity(entity, this)), comp, transform, entity);
    }
  }

  virtual void OnUpdate(Timestep ts) = 0;
  void OnUpdateRuntime(Timestep ts, int &mouseX, int &mouseY,
                       glm::vec2 &viewportSize);
  void OnUpdateEditor(Timestep ts, EditorCamera &camera, int &mouseX,
                      int &mouseY, glm::vec2 &viewportSize);
  void OnViewportResize(uint32_t width, uint32_t height);
  void OnMouseInput(float mouseX, float mouseY, bool mousePressed, Timestep ts);

  void DuplicateEntity(Entity entity);

  Entity GetHoveredEntity();

  entt::registry &GetRegistry() { return m_Registry; }
  // Ref<Framebuffre>& GetBuffer() { return m_Framebuffer;}
  // temp
  Ref<Framebuffer> m_Framebuffer;
  bool ShowBoxes = false;
  bool ShowCams = true;
  bool ShowBoxesPlay = false;

  void ReadPixelEntity(int &mouseX, int &mouseY, glm::vec2 &viewportSize);

protected:
  template <typename T> void OnComponentAdded(Entity entity, T &component);
  template <typename ShapeComp>
  void BuildRigidBodyForEntity(Entity e, TransformComponent &tr,
                               ShapeComp &src);
  template <typename TChar>
  void BuildCharacterForEntity(Entity e, TransformComponent &tr, TChar &cc);

  entt::registry m_Registry;
  uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
  float m_MouseX, m_MouseY;
  friend class Entity;
  friend class SceneSerializer;

  std::vector<entt::entity>
      m_DestroyQueue; // queued destroys flushed after update
};
} // namespace UE

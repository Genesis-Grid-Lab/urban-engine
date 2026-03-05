#pragma once

#include "Animation/Animation.h"
#include "Log.h"
#include "UUID.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/base/maths/simd_math.h"
#include <ozz/base/maths/soa_float4x4.h>
#include <ozz/base/maths/soa_transform.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include "Config.h"
#include "Renderer/Camera.h"
#include "Renderer/Font.h"
#include "Renderer/Model.h"
#include "Renderer/Skybox.h"
#include "Renderer/Texture.h"
#include "SceneCamera.h"
#include <glm/gtx/quaternion.hpp>

namespace UE {
// Rigid body "role"
enum class BodyType { Static, Dynamic, Kinematic };

struct IDComponent {
  UUID ID;

  IDComponent() = default;
  IDComponent(const IDComponent &) = default;
};

struct TagComponent {
  std::string Tag;

  TagComponent() = default;
  TagComponent(const TagComponent &) = default;
  TagComponent(const std::string &tag) : Tag(tag) {}
};

struct TransformComponent {
  glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
  glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
  glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

  TransformComponent() = default;
  TransformComponent(const TransformComponent &) = default;
  TransformComponent(const glm::vec3 &translation) : Translation(translation) {}

  glm::mat4 GetTransform() const {
    glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

    return glm::translate(glm::mat4(1.0f), Translation) * rotation *
           glm::scale(glm::mat4(1.0f), Scale);
  }

  float GetRadius() const {
    // Assuming you're transforming a unit sphere
    return glm::compMax(Scale) * 0.5f; // max(Scale.x, Scale.y, Scale.z)
  }
};

struct CameraComponent {
  SceneCamera Camera;
  bool Primary = false; // TODO: think about moving to Scene
  bool FixedAspectRatio = false;

  CameraComponent() = default;
  CameraComponent(const CameraComponent &) = default;
};

struct SkyboxComponent {
  Ref<Skybox> skybox;
  Camera *Cam;
  SkyboxComponent() = default;
  SkyboxComponent(const SkyboxComponent &) = default;
};

class ScriptableEntity;

struct NativeScriptComponent {
  ScriptableEntity *Instance = nullptr;

  ScriptableEntity *(*InstantiateScript)();
  void (*DestroyScript)(NativeScriptComponent *);

  template <typename T> void Bind() {
    InstantiateScript = []() {
      return static_cast<ScriptableEntity *>(new T());
    };
    DestroyScript = [](NativeScriptComponent *nsc) {
      delete nsc->Instance;
      nsc->Instance = nullptr;
    };
  }
};

struct ModelComponent {
  Ref<Model> ModelData;

  ModelComponent() = default;
  ModelComponent(const ModelComponent &) = default;
};

struct CubeComponent {
  glm::vec3 Color;
  CubeComponent() = default;
  CubeComponent(const CubeComponent &) = default;
};

struct LightComponent {
  glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
  LightComponent() = default;
  LightComponent(const LightComponent &) = default;
};

struct SpriteRendererComponent {
  glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
  Ref<Texture2D> Texture;
  float TilingFactor = 1.0f;

  SpriteRendererComponent() = default;
  SpriteRendererComponent(const SpriteRendererComponent &) = default;
  SpriteRendererComponent(const glm::vec4 &color) : Color(color) {}
};

struct CircleComponent {
  glm::vec4 Color{1.0f};
  float Radius = 0.0f;
  CircleComponent() = default;
  CircleComponent(const CircleComponent &) = default;
};

struct RectangleComponent {
  glm::vec4 Color{1.0f};
  RectangleComponent() = default;
  RectangleComponent(const RectangleComponent &) = default;
};

struct LineComponent {
  glm::vec4 Color;
  float Thickness;
  glm::vec2 p0, p1;
  float Order = -1;
  LineComponent() = default;
  LineComponent(const LineComponent &) = default;
};

// Animation 3D
struct AnimatorComponent {

  Model *ModelRef = nullptr;
  Animation CurrentAnimation;

  // Playback
  float Time = 0.0f;
  bool Loop = true;

  // Ozz Runtime Buffers
  ozz::unique_ptr<ozz::animation::SamplingJob::Context> Context;
  // ozz::animation::SamplingJob::Context Context;

  std::vector<ozz::math::SoaTransform> LocalTransforms;
  std::vector<ozz::math::Float4x4> ModelMatrices;
  std::vector<ozz::math::Float4x4> FinalMatrices;

  void InitFromModel(Model *model) {
    ModelRef = model;

    int joints = model->GetSkeleton()->num_joints();
    ModelMatrices.resize(joints);
    FinalMatrices.resize(joints);
  }

  void Play(Animation &&animation) {
    CurrentAnimation = std::move(animation);
    Time = 0.0f;

    auto *anim = CurrentAnimation.Get();

    int soaTracks = anim->num_soa_tracks();
    int joints = ModelRef->GetSkeleton()->num_joints();

    if (anim->num_tracks() != joints) {
      UE_CORE_ERROR("anim->num_tracks() != joints");
    }

    LocalTransforms.resize(soaTracks);
    ModelMatrices.resize(joints);
    FinalMatrices.resize(joints);

    Context = ozz::make_unique<ozz::animation::SamplingJob::Context>(joints);
    // Context.Resize(joints);

    UE_CORE_WARN("Animation SoA tracks: {}", soaTracks);
    UE_CORE_WARN("LocalTransforms size: {}", LocalTransforms.size());
  }

  AnimatorComponent() = default;
  // AnimatorComponent(const AnimatorComponent &) = default;
};

// physics 3d

struct BoxColliderComponent {
  glm::vec3 HalfSize = {0.5f, 0.5f, 0.5f};

  float Friction = 0.5f;
  float Restitution = 0.0f;

  bool IsTrigger = false;
};

struct RigidbodyComponent {
  enum class BodyType { Static = 0, Dynamic, Kinematic };

  BodyType Type = BodyType::Static;

  float Mass = 1.0f;
  float LinearDamping = 0.0f;
  float AngularDamping = 0.05f;

  bool UseGravity = true;

  int32_t BodyID;
  bool RuntimeCreated = false;
};

/// UI
struct UIElement {
  Ref<Texture2D> Texture;
  glm::vec4 Color;

  UIElement() = default;
  UIElement(const UIElement &) = default;
};

struct ButtonComponent : public UIElement {
  std::function<void()> OnClick = nullptr;
  bool Hovered = false;
  bool ClickedLastFrame = false;
  glm::vec3 OriginalScale = {120, 50, 1};
  glm::vec3 TargetScale = {120, 50, 1};
  glm::vec4 BaseColor = {1, 1, 1, 1};
  glm::vec4 CurrentColor = {1, 1, 1, 1};

  ButtonComponent() = default;
  ButtonComponent(const ButtonComponent &) = default;
};

struct TextUIComponent : public UIElement {
  Ref<Font> m_Font;
  std::string Text;
  TextUIComponent() = default;
  TextUIComponent(const TextUIComponent &) = default;
};

} // namespace UE

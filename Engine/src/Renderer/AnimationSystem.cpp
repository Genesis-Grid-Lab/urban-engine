#include "Renderer/Animation/AnimationSystem.h"
#include "Log.h"
#include "Runtime/Components.h"
#include "Runtime/Entity.h"
#include "Runtime/Scene.h"

#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/maths/soa_float4x4.h>
#include <ozz/base/maths/soa_transform.h>
// #include <ozz/io/archive.h>
// #include <ozz/io/file.h>

namespace UE {

void AnimationSystem::Update(Scene *scene, Timestep ts) {
  scene->ViewEntity<Entity, AnimatorComponent>([&ts](auto entity, auto &anim) {
    if (!anim.ModelRef)
      return;

    auto *skeleton = anim.ModelRef->GetSkeleton();
    auto *animation = anim.CurrentAnimation.Get();

    if (!animation)
      return;

    anim.Time += ts;

    float duration = animation->duration();

    if (anim.Loop)
      anim.Time = fmod(anim.Time, duration);
    else
      anim.Time = glm::min(anim.Time, duration);

    float ratio = anim.Time / duration;

    // UE_CORE_WARN("Skeleton joints: {}", skeleton->num_joints());
    // UE_CORE_WARN("Animation tracks: {}", animation->num_tracks());
    // UE_CORE_WARN("Context tracks: {}", anim.Context->max_soa_tracks());

    // Sampling
    ozz::animation::SamplingJob sampling;
    sampling.animation = animation;
    sampling.context = anim.Context.get();
    sampling.ratio = ratio;
    sampling.output = ozz::make_span(anim.LocalTransforms);

    if (!sampling.Run()) {
      UE_CORE_ERROR("Sampling failed");
      return;
    }

    // Local → Model
    ozz::animation::LocalToModelJob ltm;
    ltm.skeleton = skeleton;
    ltm.input = ozz::make_span(anim.LocalTransforms);
    ltm.output = ozz::make_span(anim.ModelMatrices);

    if (!ltm.Run()) {
      UE_CORE_ERROR("LocalToModel failed");
      return;
    }

    // Skinning matrices
    const auto &inverseBind = anim.ModelRef->GetInverseBindMatrices();

    for (size_t i = 0; i < anim.ModelMatrices.size(); i++) {
      anim.FinalMatrices[i] = anim.ModelMatrices[i] * inverseBind[i];
    }
  });
}
} // namespace UE
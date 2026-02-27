#include "Renderer/Animation/AnimationSystem.h"
#include "Runtime/Components.h"
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include "Runtime/Scene.h"
#include "Runtime/Entity.h"

namespace UE {

void AnimationSystem::Update(Scene* scene, Timestep ts)
{

    scene->ViewEntity<Entity, AnimatorComponent>([&ts](auto entity, auto &comp){        

        if (!comp.CurrentAnimation || !comp.Skeleton)
            return;

        comp.Time += ts;

        float duration = comp.CurrentAnimation->Animation.duration();
        if (comp.Loop)
            comp.Time = fmod(comp.Time, duration);
        else
            comp.Time = glm::min(comp.Time, duration);

        // --- Sampling ---
        ozz::animation::SamplingJob samplingJob;
        samplingJob.animation = &comp.CurrentAnimation->Animation;
        samplingJob.context = &comp.Context;
        samplingJob.ratio = comp.Time / duration;
        samplingJob.output = ozz::make_span(comp.LocalTransform);

        if (!samplingJob.Run())
            return;

        // --- Local to Model ---
        ozz::animation::LocalToModelJob ltmJob;
        ltmJob.skeleton = &comp.Skeleton->Skeleton;
        ltmJob.input = ozz::make_span(comp.LocalTransform);
        ltmJob.output = ozz::make_span(comp.ModelMatrices);

        ltmJob.Run();
    });

}

}
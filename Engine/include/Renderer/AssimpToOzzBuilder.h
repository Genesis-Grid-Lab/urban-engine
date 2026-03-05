#pragma once

#include "Animation/AnimData.h"
#include "Config.h"
#include <assimp/scene.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/memory/unique_ptr.h>

namespace UE {

// typedef ozz::animation::Skeleton Skeleton;
// typedef ozz::animation::Animation Animation;
// template <typename T> using OScope = ozz::unique_ptr<T>;

// ASSIMP SKELETON BUILDER

class AssimpSkeletonBuilder {
public:
  static ozz::unique_ptr<ozz::animation::Skeleton> Build(const aiScene *scene);

private:
  static void BuildJoint(const aiNode *node,
                         ozz::animation::offline::RawSkeleton::Joint &joint);
};

// ASSIMP ANIMATION BUILDER

class AssimpAnimationBuilder {
public:
  static ozz::unique_ptr<ozz::animation::Animation>
  Build(const aiScene *scene, const ozz::animation::Skeleton &skeleton,
        int animIndex);
};
} // namespace UE
#pragma once

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/io/archive.h>
#include <ozz/base/io/stream.h>

namespace UE {

struct OzzSkeleton {
  ozz::animation::Skeleton Skeleton;
};

struct OzzAnimationClip {
  ozz::animation::Animation Animation;
};

} // namespace UE
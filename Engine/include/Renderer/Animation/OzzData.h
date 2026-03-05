#pragma once

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/memory/unique_ptr.h>

namespace UE {

struct Skeleton {
  ozz::unique_ptr<ozz::animation::Skeleton> Skel;

  Skeleton() = default;
};
} // namespace UE
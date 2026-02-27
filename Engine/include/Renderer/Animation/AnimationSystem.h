#pragma once

#include "OzzAnimationAsset.h"
#include "Timestep.h"

namespace UE {

class Scene;

class AnimationSystem {
  public:
    static void Update(Scene* scene, Timestep ts);

    static Ref<OzzSkeleton> LoadSkeleton(const std::string &path) {
    auto skel = CreateRef<OzzSkeleton>();

    ozz::io::File file(path.c_str(),"rb");
    ozz::io::IArchive archive(&file);
    archive >> skel->Skeleton;

    return skel;
  }

  static Ref<OzzAnimationClip> LoadAnimation(const std::string &path) {
    auto anim = CreateRef<OzzAnimationClip>();

    ozz::io::File file(path.c_str(), "rb");
    ozz::io::IArchive archive(&file);
    archive >> anim->Animation;

    return anim;
  }
};
} // namespace UE
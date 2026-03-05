#pragma once

#include "Renderer/Animation/AnimData.h"
#include "Renderer/Animation/OzzData.h"
#include "ozz/animation/runtime/skeleton.h"
#include <assimp/scene.h>

// tmp
#include <string>

namespace UE {
class Animation {
public:
  Animation() = default;
  explicit Animation(const std::string &path,
                     const ozz::animation::Skeleton *skel);

  Animation &operator=(Animation &&other) noexcept {
    m_Animation = std::move(other.m_Animation);
    return *this;
  }

  ozz::animation::Animation *Get() const { return m_Animation.get(); }

private:
  ozz::unique_ptr<ozz::animation::Animation> m_Animation;
  const ozz::animation::Skeleton *m_Skel;
  std::map<std::string, BoneInfo> m_BoneInfoMap;
  std::unordered_map<std::string, int> m_JointMap;
  int m_BoneCounter = 0;

  void LoadFromFile(const std::string &path);
  void ProcessMesh(aiMesh *mesh, const aiScene *scene);
  void ProcessNode(aiNode *node, const aiScene *scene);
};
} // namespace UE
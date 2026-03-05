#include "Renderer/AssimpToOzzBuilder.h"
#include "Animation/AnimData.h"
#include "Log.h"
#include "assimp/anim.h"
#include "ozz/animation/offline/raw_animation.h"
#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/skeleton.h"
#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/base/maths/transform.h>

namespace UE {

static void CollectBones(const aiScene *scene,
                         std::unordered_set<std::string> &bones) {
  for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
    aiMesh *mesh = scene->mMeshes[m];

    for (unsigned int b = 0; b < mesh->mNumBones; b++) {
      bones.insert(mesh->mBones[b]->mName.C_Str());
    }
  }
}

static bool NodeContainsBone(const aiNode *node,
                             const std::map<std::string, BoneInfo> &bones) {
  if (bones.count(node->mName.C_Str()))
    return true;

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    if (NodeContainsBone(node->mChildren[i], bones))
      return true;
  }

  return false;
}

static ozz::math::Transform ConvertTransform(const aiMatrix4x4 &m) {
  aiVector3D scale;
  aiQuaternion rot;
  aiVector3D pos;

  m.Decompose(scale, rot, pos);

  ozz::math::Transform t;
  t.translation = ozz::math::Float3(pos.x, pos.y, pos.z);
  t.rotation = ozz::math::Quaternion(rot.x, rot.y, rot.z, rot.w);
  t.scale = ozz::math::Float3(scale.x, scale.y, scale.z);

  return t;
}

static float ToSeconds(double time, double ticksPerSecond) {
  return float(time / ticksPerSecond);
}

void AssimpSkeletonBuilder::BuildJoint(
    const aiNode *node, ozz::animation::offline::RawSkeleton::Joint &joint) {
  joint.name = node->mName.C_Str();
  joint.transform = ConvertTransform(node->mTransformation);

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    // const aiNode *child = node->mChildren[i];

    // std::string name = child->mName.C_Str();

    // ozz::animation::offline::RawSkeleton::Joint childJoint;

    // BuildJoint(child, childJoint);

    // joint.children.push_back(childJoint);

    joint.children.emplace_back();
    BuildJoint(node->mChildren[i], joint.children.back());
  }
}
ozz::unique_ptr<ozz::animation::Skeleton>
AssimpSkeletonBuilder::Build(const aiScene *scene) {
  ozz::animation::offline::RawSkeleton raw;

  raw.roots.emplace_back();

  BuildJoint(scene->mRootNode, raw.roots[0]);

  if (!raw.Validate()) {
    UE_CORE_ERROR("[ASSIMP->OZZ] Raw skeleton invalid");
    return nullptr;
  }

  ozz::animation::offline::SkeletonBuilder builder;

  return builder(raw);
}

// ASSIMP ANIMATION BUILDER

int FindJoint(const ozz::animation::Skeleton &skel, const std::string &name) {

  for (int i = 0; i < skel.num_joints(); i++) {
    if (name == skel.joint_names()[i])
      return i;
  }

  return -1;
}

std::string CleanNodeName(const std::string &name) {

  size_t pos = name.find("_$AssimpFbx$");
  if (pos != std::string::npos)
    return name.substr(0, pos);

  return name;
}

ozz::unique_ptr<ozz::animation::Animation>
AssimpAnimationBuilder::Build(const aiScene *scene,
                              const ozz::animation::Skeleton &skeleton,
                              int animIndex) {
  const aiAnimation *anim = scene->mAnimations[0];

  ozz::animation::offline::RawAnimation raw;

  raw.duration = anim->mDuration / anim->mTicksPerSecond;
  raw.tracks.resize(skeleton.num_joints());

  for (unsigned int c = 0; c < anim->mNumChannels; c++) {
    aiNodeAnim *channel = anim->mChannels[c];

    std::string nodeName = CleanNodeName(channel->mNodeName.C_Str());

    UE_CORE_WARN("[BUILD ANIM] chanel name: {}", nodeName);
    int joint = FindJoint(skeleton, nodeName);

    if (joint < 0)
      continue;

    auto &track = raw.tracks[joint];

    for (unsigned int i = 0; i < channel->mNumPositionKeys; i++) {
      auto &key = track.translations.emplace_back();

      key.time = channel->mPositionKeys[i].mTime / anim->mTicksPerSecond;

      aiVector3D v = channel->mPositionKeys[i].mValue;

      key.value = ozz::math::Float3(v.x, v.y, v.z);
    }

    for (unsigned int i = 0; i < channel->mNumRotationKeys; i++) {
      auto &key = track.rotations.emplace_back();

      key.time = channel->mRotationKeys[i].mTime / anim->mTicksPerSecond;

      aiQuaternion q = channel->mRotationKeys[i].mValue;

      key.value = ozz::math::Quaternion(q.x, q.y, q.z, q.w);
    }

    for (unsigned int i = 0; i < channel->mNumScalingKeys; i++) {
      auto &key = track.scales.emplace_back();

      key.time = channel->mScalingKeys[i].mTime / anim->mTicksPerSecond;

      aiVector3D s = channel->mScalingKeys[i].mValue;

      key.value = ozz::math::Float3(s.x, s.y, s.z);
    }
  }

  if (!raw.Validate()) {
    UE_CORE_ERROR("[ASSIMPTOOZZ] raw animation failed!");
    return nullptr;
  }

  ozz::animation::offline::AnimationBuilder builder;

  return builder(raw);
}
} // namespace UE
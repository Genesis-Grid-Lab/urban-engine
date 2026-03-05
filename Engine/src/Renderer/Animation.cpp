
#include "Animation/Animation.h"
#include "AssimpToOzzBuilder.h"
#include "Renderer/AssimpToGlm.h"
#include "assimp/Importer.hpp"
#include "assimp/anim.h"
#include "assimp/postprocess.h"

namespace UE {
static float ToSecond(double t, double ticks) { return float(t / ticks); }

void Animation::ProcessMesh(aiMesh *mesh, const aiScene *scene) {

  auto &boneInfoMap = m_BoneInfoMap;
  int &boneCount = m_BoneCounter;

  for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
    int boneID = -1;
    std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

    if (boneInfoMap.find(boneName) == boneInfoMap.end()) {

      BoneInfo newBoneInfo;
      newBoneInfo.id = boneCount;
      newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
          mesh->mBones[boneIndex]->mOffsetMatrix);
      boneInfoMap[boneName] = newBoneInfo;
      boneID = boneCount;
      boneCount++;
    } else {
      boneID = boneInfoMap[boneName].id;
    }
    assert(boneID != -1);
  }
}

void Animation::ProcessNode(aiNode *node, const aiScene *scene) {
  UE_PROFILE_FUNCTION();
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    ProcessMesh(mesh, scene);
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    ProcessNode(node->mChildren[i], scene);
  }
}

Animation::Animation(const std::string &path,
                     const ozz::animation::Skeleton *skel) {

  m_Skel = skel;
  LoadFromFile(path);
}

void Animation::LoadFromFile(const std::string &path) {
  Assimp::Importer importer;
  importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
  const aiScene *scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  // error checking
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode || !scene->HasAnimations()) {
    UE_CORE_ERROR("Assimp Error: {0} {1}", importer.GetErrorString(), path);
    return;
  }

  ProcessNode(scene->mRootNode, scene);
  // Build skeleton from same file

  m_Animation = AssimpAnimationBuilder::Build(scene, *m_Skel, 0);
}
} // namespace UE
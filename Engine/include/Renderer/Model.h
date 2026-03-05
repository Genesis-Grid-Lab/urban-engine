#pragma once

#include "Core/Config.h"
#include "Mesh.h"
#include "Renderer/Animation/AnimData.h"
#include "Texture.h"
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/base/maths/simd_math.h"
#include "ozz/base/memory/unique_ptr.h"
#include <assimp/scene.h>

namespace UE {

class Model {
public:
  Model(const std::string &path, bool gamma = false);
  void Draw(const Ref<Shader> &shader);
  int GetMeshCount() { return m_Meshes.size(); }
  auto &GetBoneInfoMap() { return m_BoneInfoMap; }
  int &GetBoneCount() { return m_BoneCounter; }
  std::vector<Ref<Mesh>> &GetMeshes() { return m_Meshes; }
  ozz::animation::Skeleton *GetSkeleton() const { return m_Skeleton.get(); }
  const std::vector<ozz::math::Float4x4> &GetInverseBindMatrices() const {
    return m_InverseBindMatrices;
  }
  // TO DO: Change
  std::string m_Path;

private:
  void LoadModel(const std::string &path);
  void ProcessNode(aiNode *node, const aiScene *scene);
  Ref<Mesh> ProcessMesh(aiMesh *mesh, const aiScene *scene);
  std::vector<TextureMesh> LoadMaterialTextures(aiMaterial *mat,
                                                aiTextureType type,
                                                std::string typeName);
  void SetVertexBoneDataToDefault(Vertex &vertex);
  void SetVertexBoneData(Vertex &vertex, int boneID, float weight);
  void ExtractBoneWeightForVertices(std::vector<Vertex> &vertices, aiMesh *mesh,
                                    const aiScene *scene);

private:
  std::vector<Ref<Mesh>> m_Meshes;
  std::vector<TextureMesh> m_TexturesLoaded;
  std::string m_Directory;
  bool m_GammaCorrection;
  std::map<std::string, BoneInfo> m_BoneInfoMap;
  std::unordered_map<std::string, int> m_JointMap;
  int m_BoneCounter = 0;
  ozz::unique_ptr<ozz::animation::Skeleton> m_Skeleton;
  std::vector<ozz::math::Float4x4> m_InverseBindMatrices;
};

} // namespace UE
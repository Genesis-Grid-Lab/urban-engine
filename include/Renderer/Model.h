#pragma once

#include "Core/Config.h"
#include "Mesh.h"
#include "Texture.h"
#include <assimp/scene.h>
#include "Renderer/Animation/AnimData.h"

namespace UE {

    class UE_API Model {
    public:
        Model(const std::string& path, bool gamma = false);
        void Draw(const Ref<Shader>& shader);
        int GetMeshCount(){ return m_Meshes.size();}
        auto& GetBoneInfoMap() { return m_BoneInfoMap; }
	    int& GetBoneCount() { return m_BoneCounter; }
    private:
        void LoadModel(const std::string& path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        Ref<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<TextureMesh> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
        void SetVertexBoneDataToDefault(Vertex& vertex);
        void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
        void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
    private:
        std::vector<Ref<Mesh>> m_Meshes;
        std::vector<TextureMesh> m_TexturesLoaded;
        std::string m_Directory;
        bool m_GammaCorrection;
        std::map<std::string, BoneInfo> m_BoneInfoMap;
	    int m_BoneCounter = 0;
    };

}
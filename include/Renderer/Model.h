#pragma once

#include "Core/Config.h"
#include "Mesh.h"
#include "Texture.h"
#include <assimp/scene.h>

namespace UE {

    class UE_API Model {
    public:
        Model(const std::string& path);
        void Draw(Ref<Shader>& shader);
        int GetMeshCount(){ return m_Meshes.size();}
    private:
        std::vector<Ref<Mesh>> m_Meshes;
        void LoadModel(const std::string& path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        Ref<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<TextureMesh> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
        std::vector<TextureMesh> m_TexturesLoaded;
        std::string m_Directory;
    };

}
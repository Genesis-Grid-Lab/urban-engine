#pragma once

#include "Core/Config.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace UE {

    class RaylibModelLoader
    {
    public:
        struct LoadedModel {
            Model model;
            std::vector<ModelAnimation> animations;
        };

        RaylibModelLoader();
        ~RaylibModelLoader();

        LoadedModel LoadFromFile(const std::string& path);

    private:
        Model ProcessAssimpScene(const aiScene* scene);
        Mesh ProcessAssimpMesh(aiMesh* mesh);
        ModelAnimation ProcessAssimpAnimation(const aiAnimation* animation, const aiScene* scene);

        Vector3 AiVector3ToVector3(const aiVector3D& v);
        Quaternion AiQuaternionToQuaternion(const aiQuaternion& q);
    };
}

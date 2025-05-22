#include "uepch.h"
#include "Renderer/Model.h"
#include <raymath.h>
#include "Core/Log.h"

namespace UE {
    RaylibModelLoader::RaylibModelLoader() {}
    RaylibModelLoader::~RaylibModelLoader() {}

    RaylibModelLoader::LoadedModel RaylibModelLoader::LoadFromFile(const std::string& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_CalcTangentSpace |
            aiProcess_LimitBoneWeights |
            aiProcess_GenSmoothNormals |
            aiProcess_ImproveCacheLocality |
            aiProcess_SortByPType |
            aiProcess_FlipUVs
        );

        if (!scene || !scene->HasMeshes())
        {
            // TraceLog(LOG_ERROR, "Assimp error: %s", importer.GetErrorString());
            UE_CORE_WARN("Assimp error: {}", importer.GetErrorString());
            return {};
        }

        LoadedModel result;
        result.model = ProcessAssimpScene(scene);

        if (scene->HasAnimations())
        {
            for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
            {
                result.animations.push_back(ProcessAssimpAnimation(scene->mAnimations[i], scene));
            }
        }

        return result;
    }

    Model RaylibModelLoader::ProcessAssimpScene(const aiScene* scene)
    {
        Model model = {0};
        model.meshCount = scene->mNumMeshes;
        model.meshes = (Mesh*)RL_CALLOC(model.meshCount, sizeof(Mesh));
        model.materialCount = scene->mNumMaterials;
        model.materials = (Material*)RL_CALLOC(1, sizeof(Material)); // Placeholder
        // model.materials[0] = LoadMaterialDefault();
        for(int i = 0; i < model.materialCount; i++){
            UE_CORE_WARN("{}", i);
            model.materials[i] = LoadMaterialDefault();
        }
        model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));

        for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
        {
            model.meshes[i] = ProcessAssimpMesh(scene->mMeshes[i]);
            model.meshMaterial[i] = 0; // All meshes use default material for now
            UE_CORE_INFO("Mesh {}: vertices = {}, triangles = {}", i, model.meshes[i].vertexCount, model.meshes[i].triangleCount);
        }

        return model;
    }

    Mesh RaylibModelLoader::ProcessAssimpMesh(aiMesh* mesh)
    {
        Mesh rmesh = {0};
        rmesh.vertexCount = mesh->mNumVertices;
        rmesh.triangleCount = mesh->mNumFaces;

        rmesh.vertices = (float*)RL_CALLOC(rmesh.vertexCount * 3, sizeof(float));
        rmesh.texcoords = (float*)RL_CALLOC(rmesh.vertexCount * 2, sizeof(float));
        rmesh.normals = (float*)RL_CALLOC(rmesh.vertexCount * 3, sizeof(float));

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            rmesh.vertices[i * 3 + 0] = mesh->mVertices[i].x;
            rmesh.vertices[i * 3 + 1] = mesh->mVertices[i].y;
            rmesh.vertices[i * 3 + 2] = mesh->mVertices[i].z;
            
            if (mesh->HasNormals())
            {
                rmesh.normals[i * 3 + 0] = mesh->mNormals[i].x;
                rmesh.normals[i * 3 + 1] = mesh->mNormals[i].y;
                rmesh.normals[i * 3 + 2] = mesh->mNormals[i].z;
            }
            
            if (mesh->HasTextureCoords(0))
            {
                rmesh.texcoords[i * 2 + 0] = mesh->mTextureCoords[0][i].x;
                rmesh.texcoords[i * 2 + 1] = mesh->mTextureCoords[0][i].y;
            }            
        }

        UploadMesh(&rmesh, false);
        return rmesh;
    }

    ModelAnimation RaylibModelLoader::ProcessAssimpAnimation(const aiAnimation* anim, const aiScene* scene)
    {
        ModelAnimation rAnim = {0};
        rAnim.frameCount = (int)anim->mDuration;

        rAnim.boneCount = anim->mNumChannels;
        rAnim.bones = (BoneInfo*)RL_CALLOC(rAnim.boneCount, sizeof(BoneInfo));
        rAnim.framePoses = (Transform**)RL_CALLOC(rAnim.frameCount, sizeof(Transform*));

        for (int i = 0; i < rAnim.frameCount; i++)
        {
            rAnim.framePoses[i] = (Transform*)RL_CALLOC(rAnim.boneCount, sizeof(Transform));
        }

        for (unsigned int i = 0; i < anim->mNumChannels; ++i)
        {
            aiNodeAnim* channel = anim->mChannels[i];
            strcpy(rAnim.bones[i].name, channel->mNodeName.C_Str());
            rAnim.bones[i].parent = -1; // Simple flat hierarchy for now

            for (int frame = 0; frame < rAnim.frameCount; frame++)
            {
                Transform& t = rAnim.framePoses[frame][i];

                if (frame < (int)channel->mNumPositionKeys)
                    t.translation = AiVector3ToVector3(channel->mPositionKeys[frame].mValue);
                else
                    t.translation = { 0, 0, 0 };

                if (frame < (int)channel->mNumRotationKeys)
                    t.rotation = AiQuaternionToQuaternion(channel->mRotationKeys[frame].mValue);
                else
                    t.rotation = QuaternionIdentity();

                if (frame < (int)channel->mNumScalingKeys)
                    t.scale = AiVector3ToVector3(channel->mScalingKeys[frame].mValue);
                else
                    t.scale = { 1, 1, 1 };
            }
        }

        return rAnim;
    }

    Vector3 RaylibModelLoader::AiVector3ToVector3(const aiVector3D& v)
    {
        return { v.x, v.y, v.z };
    }

    Quaternion RaylibModelLoader::AiQuaternionToQuaternion(const aiQuaternion& q)
    {
        return { q.x, q.y, q.z, q.w };
    }
}

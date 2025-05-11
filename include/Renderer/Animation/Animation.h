#pragma once

#include "Core/Config.h"
#include <assimp/scene.h>
#include "AnimData.h"
#include "Renderer/Model.h"
#include "Bone.h"

namespace UE {

    struct AssimpNodeData{
        glm::mat4 Transformation;
        std::string Name;
        int ChildrenCount;
        std::vector<AssimpNodeData> Children;
    };

    class UE_API Animation{
    public:
        Animation() = default;
        Animation(const std::string& animationPath, Ref<Model> model = nullptr);
        ~Animation() = default;

        void SetModel(Ref<Model> model);

        Bone* FindBone(const std::string& name);
        inline float GetTicksPerSecond() { return m_TicksPerSecond; }
        inline float GetDuration() { return m_Duration;}
        inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
        inline const std::map<std::string,BoneInfo>& GetBoneIDMap() 
        { 
            return m_BoneInfoMap;
        }
    private:
        void ReadMissingBones(const aiAnimation* animation, Ref<Model>& model);
        void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
    private:
        float m_Duration;
        int m_TicksPerSecond;
        std::vector<Bone> m_Bones;
        AssimpNodeData m_RootNode;
        std::map<std::string, BoneInfo> m_BoneInfoMap;

        const aiAnimation* m_AssimpAnimation = nullptr;        
        Ref<Model> m_Model;
    };
}
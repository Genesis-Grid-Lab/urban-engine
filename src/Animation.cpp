#include "uepch.h"
#include "Renderer/Animation/Animation.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace UE {

    Animation::Animation(const std::string& animationPath, Ref<Model> model):m_AssimpAnimation(nullptr), m_Model(model){
        Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        if(scene == nullptr){
            UE_CORE_ERROR("Failed to load {}", animationPath);
        }        
        		
		m_AssimpAnimation = scene->mAnimations[0];
        for (int i = 0; i < m_AssimpAnimation->mNumChannels; i++) {
            auto channel = m_AssimpAnimation->mChannels[i];
            UE_CORE_INFO("Animation Channel: {}", channel->mNodeName.C_Str());
        }  
              
		m_Duration = m_AssimpAnimation->mDuration;
		m_TicksPerSecond = m_AssimpAnimation->mTicksPerSecond;
        UE_CORE_INFO("Loaded animation: {} | Duration: {} | TicksPerSecond: {}", animationPath, m_Duration, m_TicksPerSecond);
		// aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		// globalTransformation = globalTransformation.Inverse();
		ReadHierarchyData(m_RootNode, scene->mRootNode);
		// ReadMissingBones(animation, *model);        
        if (m_Model) {
            SetModel(m_Model); // safe call
        } else {
            UE_CORE_WARN("Animation constructed without model. Call SetModel(model) before playing.");
        }
    }

    void Animation::SetModel(Ref<Model> model) {
        if (!m_AssimpAnimation || m_Bones.size() > 0) {
            UE_CORE_ERROR("Cannot bind model: No animation loaded.");
            return;
        }
        if (!model) {
            UE_CORE_ERROR("Cannot bind null model.");
            return;
        }

        m_Model = model;        
        ReadMissingBones(m_AssimpAnimation, m_Model);
    }

    Bone* Animation::FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

    void Animation::ReadMissingBones(const aiAnimation* animation, Ref<Model>& model)
	{        
		int size = animation->mNumChannels;

		auto& boneInfoMap = model->GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		int& boneCount = model->GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].id = boneCount;
				boneCount++;
			}
			m_Bones.push_back(Bone(channel->mNodeName.data,
				boneInfoMap[channel->mNodeName.data].id, channel));
		}

		m_BoneInfoMap = boneInfoMap;
	}

    void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		assert(src);

		dest.Name = src->mName.data;
		dest.Transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.ChildrenCount = src->mNumChildren;

		for (int i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHierarchyData(newData, src->mChildren[i]);
			dest.Children.push_back(newData);
		}
	}
}
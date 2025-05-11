#include "Renderer/Animation/Animator.h"
#include <assimp/Importer.hpp>
#include "Renderer/Animation/Bone.h"

namespace UE {

    Animator::Animator(){
        m_CurrentTime = 0.0;
        m_FinalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    Animator::Animator(Ref<Animation> animation)
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = animation;

		m_FinalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}

    void Animator::UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
            float tocksPerSecond = m_CurrentAnimation->GetTicksPerSecond();
			m_CurrentTime += tocksPerSecond * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
        
	}

    void Animator::PlayAnimation(Ref<Animation> pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

    void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
	{
        if (!node || !m_CurrentAnimation)
		    return;

        std::string nodeName = node->Name;
        glm::mat4 nodeTransform = node->Transformation;

        Bone* bone = m_CurrentAnimation->FindBone(nodeName);
        if (bone)
        {
            bone->Update(m_CurrentTime);
            nodeTransform = bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        const auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        auto it = boneInfoMap.find(nodeName);
        if (it != boneInfoMap.end())
        {
            int index = it->second.id;
            glm::mat4 offset = it->second.offset;
            if (index >= 0 && index < m_FinalBoneMatrices.size())
                m_FinalBoneMatrices[index] = globalTransformation * offset;
            else
                UE_CORE_WARN("[Animator] Invalid bone index: {}" ,index);
        }

        for (int i = 0; i < node->ChildrenCount; i++)
        {
            CalculateBoneTransform(&node->Children[i], globalTransformation);
        }
	}
}
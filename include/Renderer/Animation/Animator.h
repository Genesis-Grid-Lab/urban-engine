#pragma once

#include "Core/Config.h"
#include "Animation.h"
#include <assimp/scene.h>

namespace UE {

    class UE_API Animator{
    public:
        Animator();
        Animator(Ref<Animation> animation);
        void UpdateAnimation(float dt);
        void PlayAnimation(Ref<Animation> pAnimation);
        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
        std::vector<glm::mat4> GetFinalBoneMatrices()
        {
            return m_FinalBoneMatrices;
        }

        Ref<Animation> GetCurrentAnimation(){ return m_CurrentAnimation;}
    private:
        std::vector<glm::mat4> m_FinalBoneMatrices;
        Ref<Animation> m_CurrentAnimation;
        float m_CurrentTime;
        float m_DeltaTime;
    };
}
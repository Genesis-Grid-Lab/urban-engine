#pragma once

#include "Core/Config.h"

namespace UE {

    struct BoneInfo{
        /*id is index in finalBoneMatrices*/
        int id;

        /*offset matrix transforms vertex from model space to bone space*/
        glm::mat4 offset;
    };
}
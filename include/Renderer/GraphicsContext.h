#pragma once

#include "Config.h"
#include <glad/glad.h>

namespace UE {

    class UE_API GraphicsContext{
    public:
        GraphicsContext();

        void Init(GLADloadproc proc);        

    private:
    };
}
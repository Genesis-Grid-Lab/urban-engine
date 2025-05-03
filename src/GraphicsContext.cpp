#include "Renderer/GraphicsContext.h"
#include "UE_Assert.h"

namespace UE {

    GraphicsContext::GraphicsContext(){}                

    void GraphicsContext::Init(GLADloadproc proc){

        int status = gladLoadGLLoader(proc);
		UE_CORE_ASSERT(status, "Failed to initialize Glad!");

		UE_CORE_INFO("OpenGL Info:");
		UE_CORE_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		UE_CORE_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
		UE_CORE_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));

		UE_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "Engine requires at least OpenGL version 4.5!");
    }
}
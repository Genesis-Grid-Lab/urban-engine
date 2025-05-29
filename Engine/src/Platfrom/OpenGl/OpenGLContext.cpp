#include "uepch.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Core/UE_Assert.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace UE {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		UE_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void OpenGLContext::Init()
	{
		// UE_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		UE_CORE_ASSERT(status, "Failed to initialize Glad!");

		UE_CORE_INFO("OpenGL Info:");
		UE_CORE_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		UE_CORE_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
		UE_CORE_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));

		UE_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "FunShot requires at least OpenGL version 4.5!");
	}

	void OpenGLContext::SwapBuffers()
	{
		// UE_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

}

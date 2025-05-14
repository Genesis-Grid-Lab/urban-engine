#pragma once
#pragma warning(disable : 4996)


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "PlatformDetection.h"


#ifdef UE_DEBUG
	#if defined(UE_PLATFORM_WINDOWS)
		#define UE_DEBUGBREAK() __debugbreak()
	#elif defined(UE_PLATFORM_LINUX)
		#include <signal.h>
		#define UE_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define UE_ENABLE_ASSERTS
#else
	#define UE_DEBUGBREAK()
#endif

#define UE_EXPAND_MACRO(x) x
#define UE_STRINGIFY_MACRO(x) #x

// Define UE_API for import/export depending on platform and usage
#if defined(UE_PLATFORM_WINDOWS)
    #ifdef UE_EXPORT    
    #ifdef _MSC_VER
        // #define UE_API __declspec(dllexport)
    #else
        // #define UE_API __attribute__((visibility("default")))
    #endif
    #endif

    #ifdef UE_IMPORT           
    #ifdef _MSC_VER
        // #define UE_API __declspec(dllimport)
    #else
        #define UE_API
    #endif
    #endif
#else
    // Linux/macOS: only need to set visibility when exporting
    #ifdef UE_EXPORT
        // #define UE_API __attribute__((visibility("default")))
    #else
        #define UE_API
    #endif
#endif

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 1280

namespace UE {

    //--------------------- Scope = unique pointer --------------------
    template<typename T>
    using Scope = std::unique_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args){

        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    //--------------------- Ref = shared pointer -----------------------
    template<typename T>
    using Ref = std::shared_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args){

        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}

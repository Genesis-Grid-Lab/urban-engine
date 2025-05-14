#pragma once

#include "Config.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace UE {

    class  Log{
    public:
        static void Init();
        
        static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger;}
        static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger;}
    private:
        static Ref<spdlog::logger> s_CoreLogger;
        static Ref<spdlog::logger> s_ClientLogger;
    };
}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternio)
{
	return os << glm::to_string(quaternio);
}

// Core log macros
#define UE_CORE_TRACE(...)    ::UE::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define UE_CORE_INFO(...)     ::UE::Log::GetCoreLogger()->info(__VA_ARGS__)
#define UE_CORE_WARN(...)     ::UE::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define UE_CORE_ERROR(...)    ::UE::Log::GetCoreLogger()->error(__VA_ARGS__)
#define UE_CORE_CRITICAL(...) ::UE::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define UE_TRACE(...)         ::UE::Log::GetClientLogger()->trace(__VA_ARGS__)
#define UE_INFO(...)          ::UE::Log::GetClientLogger()->info(__VA_ARGS__)
#define UE_WARN(...)          ::UE::Log::GetClientLogger()->warn(__VA_ARGS__)
#define UE_ERROR(...)         ::UE::Log::GetClientLogger()->error(__VA_ARGS__)
#define UE_CRITICAL(...)      ::UE::Log::GetClientLogger()->critical(__VA_ARGS__)
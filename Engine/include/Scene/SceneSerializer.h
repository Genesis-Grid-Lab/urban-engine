#pragma once

#include "Scene.h"

namespace UE {
	static std::string RigidBody3DBodyTypeToString(JPH::EMotionType bodyType)
	{
		switch (bodyType)
		{
			case JPH::EMotionType::Static:    return "Static";
			case JPH::EMotionType::Dynamic:   return "Dynamic";
			case JPH::EMotionType::Kinematic: return "Kinematic";
		}

		UE_CORE_ASSERT(false, "Unknown body type");
		return {};
	}

	static JPH::EMotionType RigidBody3DBodyTypeFromString(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static")    return JPH::EMotionType::Static;
		if (bodyTypeString == "Dynamic")   return JPH::EMotionType::Dynamic;
		if (bodyTypeString == "Kinematic") return JPH::EMotionType::Kinematic;
	
		UE_CORE_ASSERT(false, "Unknown body type");
		return JPH::EMotionType::Static;
	}

	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);
	private:
		Ref<Scene> m_Scene;
	};

}

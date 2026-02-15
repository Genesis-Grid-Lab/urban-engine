#pragma once

#include "Scene.h"
#include "Runtime/Components.h"

namespace UE {
  static std::string RigidBody3DBodyTypeToString(BodyType bodyType)
	{
		switch (bodyType)
		{
			case BodyType::Static:    return "Static";
			case BodyType::Dynamic:   return "Dynamic";
			case BodyType::Kinematic: return "Kinematic";
		}

		UE_CORE_ASSERT(false, "Unknown body type");
		return {};
	}

	static BodyType RigidBody3DBodyTypeFromString(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static")    return BodyType::Static;
		if (bodyTypeString == "Dynamic")   return BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return BodyType::Kinematic;
	
		UE_CORE_ASSERT(false, "Unknown body type");
		return BodyType::Static;
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

#include "uepch.h"
#include "Scene/SceneSerializer.h"

#include "Scene/Entity.h"
#include "Scene/Components.h"
#include "Core/UE_Assert.h"
#include <fstream>
#include "Auxiliaries/YamlConvert.h"

namespace UE {

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}	

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		UE_CORE_ASSERT(entity.HasComponent<IDComponent>());

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; // TransformComponent
		}

        if (entity.HasComponent<CubeComponent>())
		{
			out << YAML::Key << "CubeComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.GetComponent<CubeComponent>();
			out << YAML::Key << "Color" << YAML::Value << tc.Color;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<ModelComponent>())
		{
			out << YAML::Key << "ModelComponent";
			out << YAML::BeginMap; // ModelComponent

			auto& modelComponent = entity.GetComponent<ModelComponent>();
			auto& model = modelComponent.ModelData;

			out << YAML::Key << "Model" << YAML::Value;
			out << YAML::BeginMap; // Model
			out << YAML::Key << "Path" << YAML::Value << model->m_Path;
			out << YAML::EndMap; // Model

			out << YAML::EndMap; // ModelComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if(entity.HasComponent<RigidbodyComponent>()){

			auto& bodyComp = entity.GetComponent<RigidbodyComponent>();
			out << YAML::Key << "RigidbodyComponent";
			out << YAML::BeginMap; // RigidbodyComponent
			out << YAML::Key << "Type" << YAML::Value << RigidBody3DBodyTypeToString(bodyComp.Type);
			out << YAML::Key << "Layer" << YAML::Value << bodyComp.Layer;
			out << YAML::Key << "Velocity" << YAML::Value << bodyComp.Velocity;
			out << YAML::EndMap; // RigidbodyComponent
		}

		if(entity.HasComponent<BoxShapeComponent>()){
			out << YAML::Key << "BoxShapeComponent";
			out << YAML::BeginMap; // BoxShapeComponent
			
			out << YAML::EndMap; // BoxShapeComponent
		}

		if(entity.HasComponent<SphereShapeComponent>()){
			out << YAML::Key << "SphereShapeComponent";
			out << YAML::BeginMap; // SphereShapeComponent
			
			out << YAML::EndMap; // SphereShapeComponent
		}

		// if (entity.HasComponent<Rigidbody2DComponent>())
		// {
		// 	out << YAML::Key << "Rigidbody2DComponent";
		// 	out << YAML::BeginMap; // Rigidbody2DComponent

		// 	auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
		// 	out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
		// 	out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;

		// 	out << YAML::EndMap; // Rigidbody2DComponent
		// }

		// if (entity.HasComponent<BoxCollider2DComponent>())
		// {
		// 	out << YAML::Key << "BoxCollider2DComponent";
		// 	out << YAML::BeginMap; // BoxCollider2DComponent

		// 	auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
		// 	out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
		// 	out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
		// 	out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
		// 	out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
		// 	out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
		// 	out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;

		// 	out << YAML::EndMap; // BoxCollider2DComponent
		// }

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				return;

			SerializeEntity(out, entity);
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		// Not implemented
		UE_CORE_ASSERT(false);
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		UE_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				UE_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					// Entities always have transforms
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

                auto cubeComponent = entity["CubeComponent"];
				if (cubeComponent)
				{
					// Entities always have transforms
					auto& src = deserializedEntity.AddComponent<CubeComponent>();
					src.Color = cubeComponent["Color"].as<glm::vec3>();					
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					auto& cameraProps = cameraComponent["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

					cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto modelComponent = entity["ModelComponent"];
				if (modelComponent)
				{
					auto& mc = deserializedEntity.AddComponent<ModelComponent>();
					auto& modelProps = modelComponent["Model"];
					Ref<Model> tempModel = CreateRef<Model>(modelProps["Path"].as<std::string>());
					mc.ModelData = tempModel;
					
				}

				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
					src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
				}

				auto rigidBodyComp = entity["RigidbodyComponent"];
				if(rigidBodyComp){
					auto& src = deserializedEntity.AddComponent<RigidbodyComponent>();
					src.Type = RigidBody3DBodyTypeFromString(rigidBodyComp["Type"].as<std::string>());
					src.Layer = rigidBodyComp["Layer"].as<unsigned int>();
					src.Velocity = rigidBodyComp["Velocity"].as<glm::vec3>();
				}

				auto boxShapeComp = entity["BoxShapeComponent"];
				if(boxShapeComp){
					auto& src = deserializedEntity.AddComponent<BoxShapeComponent>();					
				}

				auto sphereShapeComp = entity["SphereShapeComponent"];
				if(sphereShapeComp){
					auto& src = deserializedEntity.AddComponent<SphereShapeComponent>();					
				}
				
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		// Not implemented
		UE_CORE_ASSERT(false);
		return false;
	}

}

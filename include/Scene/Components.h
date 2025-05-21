#pragma once

#include "Core/Config.h"
#include "Core/UUID.h"

namespace UE {
    struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	struct TransformComponent
	{
		Vector3 Translation = { 0.0f, 0.0f, 0.0f };
		Vector3 Rotation = { 0.0f, 0.0f, 0.0f };
		Vector3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Vector3& translation)
			: Translation(translation) {}
		
	};

    struct PlaneComponent{
        Vector2 Size;
        Color color;
        PlaneComponent() = default;
        PlaneComponent(PlaneComponent&) = default;
    };

	struct ModelComponent{
		Model mModel;
		ModelComponent() = default;
		ModelComponent(ModelComponent&) = default;
	};

}
#pragma once

#include <glm/glm.hpp>
#include "Core/Config.h"

namespace UE::Math {

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

}

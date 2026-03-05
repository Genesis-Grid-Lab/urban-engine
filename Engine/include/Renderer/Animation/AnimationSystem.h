#pragma once

#include "Timestep.h"

namespace UE {

class Scene;

class AnimationSystem {
public:
  static void Update(Scene *scene, Timestep ts);
};

} // namespace UE
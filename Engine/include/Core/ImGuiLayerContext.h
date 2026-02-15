#pragma once

#include "Config.h"
#include <imgui.h>

namespace UE {

  class ImGuiLayerContext {
  public:
    virtual ~ImGuiLayerContext() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;

    virtual void NewFrame() = 0;
    virtual void EndFrame() = 0;
  };
}

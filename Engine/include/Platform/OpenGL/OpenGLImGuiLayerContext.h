#pragma once

#include "ImGuiLayerContext.h"

namespace UE {

  class OpenGLImGuiLayerContext : public ImGuiLayerContext {
  public:
    virtual ~OpenGLImGuiLayerContext() = default;

    virtual void Init() override;
    virtual void Shutdown() override;

    virtual void NewFrame() override;
    virtual void EndFrame() override;
  };
}

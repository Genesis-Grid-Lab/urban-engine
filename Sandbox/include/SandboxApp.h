#pragma once
#include "Application.h"
#include <PingPong.h>
#include <UrbanEngine.h>

class SandboxApp : public Application {
public:
  SandboxApp(ApplicationCommandLineArgs args)
      : Application("SandBox", {now_width, now_height}, args) {
    PushLayer(new PingPong());
  }
};
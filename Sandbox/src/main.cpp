#include "SandboxApp.h"
#include <Core/EntryPoint.h>
#include <UrbanEngine.h>

UE::Application *UE::CreateApplication(UE::ApplicationCommandLineArgs args) {
  return new SandboxApp(args);
}
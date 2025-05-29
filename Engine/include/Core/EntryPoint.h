#pragma once

#include "Application.h"

extern UE::Application* UE::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv){
    
    UE::Log::Init();

    UE_CORE_INFO("Loading Application");

    UE_PROFILE_BEGIN_SESSION("Startup", "UEProfile-Startup.json");
    auto app = UE::CreateApplication({ argc, argv });
    UE_PROFILE_END_SESSION();

    UE_PROFILE_BEGIN_SESSION("Runtime", "UEProfile-Runtime.json");
    app->Run();
    UE_PROFILE_END_SESSION();

    UE_CORE_INFO("Shuting down");

    UE_PROFILE_BEGIN_SESSION("Shutdown", "UEProfile-Shutdown.json");
    delete app;
    UE_PROFILE_END_SESSION();

    return 0;
}
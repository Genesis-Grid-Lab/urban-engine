#pragma once

#include "Application.h"

extern UE::Application* UE::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv){
    
    UE::Log::Init();

    UE_CORE_INFO("Loading Application");

    auto app = UE::CreateApplication({ argc, argv });

    app->Run();

    UE_CORE_INFO("Shuting down");
    delete app;

    return 0;
}
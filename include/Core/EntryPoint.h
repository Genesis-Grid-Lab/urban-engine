#pragma once

#include "Core/Application.h"

extern UE::Application* UE::CreateApplication();

int main(int argc, char** argv){
    
    UE::Log::Init();

    UE_CORE_INFO("Loading Application");

    auto app = UE::CreateApplication();

    app->Run();

    UE_CORE_INFO("Shuting down");
    delete app;

    return 0;
}
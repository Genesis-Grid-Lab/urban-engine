#include <UrbanEngine.h>
#include <Core/EntryPoint.h>
#include "EditorApp.h"

UE::Application* UE::CreateApplication(UE::ApplicationCommandLineArgs args){

    return new EditorApp(args);
}
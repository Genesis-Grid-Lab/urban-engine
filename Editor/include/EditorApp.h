#pragma once
#include <UrbanEngine.h>
#include "EditorLayer.h"

#define now_width 1600
#define now_height 900

using namespace UE;
class EditorApp : public Application {
public:
    EditorApp(ApplicationCommandLineArgs args)
        :Application("EbonyEditor",{now_width, now_height}, args){
            PushLayer(new EditorLayer({now_width, now_height}));
        }

    ~EditorApp(){}
};
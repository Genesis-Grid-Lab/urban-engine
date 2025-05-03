#pragma once

#include "Core/Layer.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

struct ImGuiContext;

namespace UE {

    class UE_API ImGuiLayer : public Layer {
    public:
        ImGuiLayer();
        ~ImGuiLayer() = default;
    
        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnEvent(Event& e) override;
    
        void Begin();
        void End();
        ImGuiContext* GetContext();
    
        void BlockEvents(bool block) { m_BlockEvents = block;}
    
        void SetDarkThemeColors();
    private:
        bool m_BlockEvents = true;
    };
}
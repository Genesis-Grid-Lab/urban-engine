#pragma once

#include <UrbanEngine.h>
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "ImGui/Console.h"

using namespace UE;

class EditorLayer : public Layer {
public:
    EditorLayer(const glm::vec2& size);

    virtual void OnAttach() override;
    virtual void OnUpdate(Timestep ts) override;
    virtual void OnEvent(Event& e) override;
    virtual void OnImGuiRender() override;
private:
    bool OnKeyPressed(KeyPressedEvent& e);
    bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

    void NewScene();
    void OpenScene();
    void OpenScene(const std::filesystem::path& path);
    void SaveScene();
    void SaveSceneAs();

    void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

    void OnScenePlay();
    void OnSceneStop();

    void OnDuplicateEntity();

    // UI Panels
    void UI_Toolbar();
private:
    glm::vec2 m_Size;
    bool m_ViewportFocused = false, m_ViewportHovered = false;
    glm::vec2 m_ViewportBounds[2];
    Ref<Scene> m_ActiveScene;
    Ref<Scene> m_EditorScene;
    glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
    Entity m_SelectionContext;
    EditorCamera m_EditorCamera;
    std::string filePath;
    std::string filePathName;
    int m_GizmoType = -1;

    enum class SceneState
    {
        Edit = 0, Play = 1
    };
    SceneState m_SceneState = SceneState::Edit;

    std::filesystem::path m_EditorScenePath;
    AppConsole console;

    // Panels
    SceneHierarchyPanel m_SceneHierarchyPanel;
    ContentBrowserPanel m_ContentBrowserPanel;

    // Editor resources
    Ref<Texture2D> m_IconPlay, m_IconStop;
};
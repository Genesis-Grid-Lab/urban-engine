#include "uepch.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Scene/Components.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo.h>

#include <cstring>

/* The Microsoft C++ compiler is non-compliant with the C++ standard and needs
 * the following definition to disable a security warning on std::strncpy().
 */
#ifdef _MSVC_LANG
  #define _CRT_SECURE_NO_WARNINGS
#endif



namespace UE {

// ------------------------
// Small UI helpers
// ------------------------
static bool DrawVec3Control(const char* label, glm::vec3& values,
                            float resetValue = 0.0f, float columnWidth = 100.0f)
{
    bool changed = false;

    ImGui::PushID(label);
    ImGui::PushID(&values);                  // <— unique per field instance

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::TextUnformatted(label);
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    const float lineHeight = ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
    const ImVec2 btn = { lineHeight + 3.0f, lineHeight };

    auto axis = [&](const char* axisText, float& v, const ImVec4& col, const char* dragID)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, col);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ col.x + 0.1f, col.y + 0.1f, col.z + 0.1f, col.w });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, col);
        if (ImGui::Button(axisText, btn)) { v = resetValue; changed = true; }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        changed |= ImGui::DragFloat(dragID, &v, 0.1f);   // <— unique per axis
        ImGui::PopItemWidth();
        ImGui::SameLine();
    };

    axis("X", values.x, ImVec4{0.8f, 0.1f, 0.15f, 1.0f}, "##X");
    axis("Y", values.y, ImVec4{0.2f, 0.7f, 0.2f, 1.0f},  "##Y");
    axis("Z", values.z, ImVec4{0.1f, 0.25f, 0.8f, 1.0f}, "##Z");

    ImGui::PopStyleVar();
    ImGui::Columns(1);

    ImGui::PopID(); // &values
    ImGui::PopID(); // label
    return changed;
}


// Generic foldout for components with add/remove menu on the right
// uiFn(entity, component) should return true if it changed something

template<typename T, typename UIFunc>
static void DrawComponent(const char* name, Entity entity, UIFunc uiFn, bool removable = true)
{
    if (!entity.HasComponent<T>())
        return;

    auto& comp = entity.GetComponent<T>();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

    ImVec2 contentRegion = ImGui::GetContentRegionAvail();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
    bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), flags, "%s", name);
    ImGui::PopStyleVar();

    // settings button on the right
    float lineHeight = ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImGui::SameLine(contentRegion.x - lineHeight * 0.5f);
    if (ImGui::Button("...", ImVec2{ lineHeight, lineHeight }))
        ImGui::OpenPopup("ComponentSettings");

    bool removeComponent = false;
    if (ImGui::BeginPopup("ComponentSettings"))
    {
        if (removable && ImGui::MenuItem("Remove component"))
            removeComponent = true;
        ImGui::EndPopup();
    }

    if (open)
    {
        uiFn(entity, comp);
        ImGui::TreePop();
    }

    if (removeComponent)
        entity.RemoveComponent<T>();
}

// ------------------------
// SceneHierarchyPanel
// ------------------------
void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
{
  m_SelectionContext = entity;
}
SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
{
    SetContext(context);
}

void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
{
    m_Context = context;
    m_SelectionContext = {};
}

void SceneHierarchyPanel::OnImGuiRender()
{
    ImGui::Begin("Scene Hierarchy");

    // List entities
    m_Context->GetRegistry().each([&](auto eID)
    {
        Entity entity{ eID, m_Context.get() };
        DrawEntityNode(entity);
    });

    // Blank-space context menu
    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
        m_SelectionContext = {};

    if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::MenuItem("Create Empty"))
            m_SelectionContext = m_Context->CreateEntity("Empty");
        ImGui::EndPopup();
    }

    ImGui::End();

    ImGui::Begin("Properties");
    if (m_SelectionContext)
        DrawComponents(m_SelectionContext);
    ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(Entity entity)
{
    auto& tag = entity.GetComponent<TagComponent>().Tag;

    ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
    flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

    bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());
    if (ImGui::IsItemClicked())
        m_SelectionContext = entity;

    bool entityDeleted = false;
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Duplicate"))
            m_Context->DuplicateEntity(entity);
        if (ImGui::MenuItem("Delete"))
            entityDeleted = true;
        ImGui::EndPopup();
    }

    if (opened)
    {
        // (Children would be drawn here if you support hierarchy)
        ImGui::TreePop();
    }

    if (entityDeleted)
    {
        if (m_SelectionContext == entity)
            m_SelectionContext = {};
        m_Context->DestroyEntity(entity);
    }
}

static void DrawAddComponentPopup(Entity entity)
{
    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        if (!entity.HasComponent<CameraComponent>())
        {
            if (ImGui::MenuItem("Camera")) { entity.AddComponent<CameraComponent>(); ImGui::CloseCurrentPopup(); }
        }
        if (!entity.HasComponent<RigidbodyComponent>() && !entity.HasComponent<CharacterComponent>())
        {
            if (ImGui::MenuItem("Rigidbody")) { entity.AddComponent<RigidbodyComponent>(); ImGui::CloseCurrentPopup(); }
        }
        if (!entity.HasComponent<CharacterComponent>() && !entity.HasComponent<RigidbodyComponent>())
        {
            if (ImGui::MenuItem("Character")) { entity.AddComponent<CharacterComponent>(); ImGui::CloseCurrentPopup(); }
        }
        if (!entity.HasComponent<BoxShapeComponent>())
        {
            if (ImGui::MenuItem("Box Collider")) { entity.AddComponent<BoxShapeComponent>(); ImGui::CloseCurrentPopup(); }
        }
        if (!entity.HasComponent<SphereShapeComponent>())
        {
            if (ImGui::MenuItem("Sphere Collider")) { entity.AddComponent<SphereShapeComponent>(); ImGui::CloseCurrentPopup(); }
        }
        if (!entity.HasComponent<NativeScriptComponent>())
        {
            if (ImGui::MenuItem("Native Script")) { entity.AddComponent<NativeScriptComponent>(); ImGui::CloseCurrentPopup(); }
        }

        ImGui::EndPopup();
    }
}

void SceneHierarchyPanel::DrawComponents(Entity entity)
{
    // Tag
    if (entity.HasComponent<TagComponent>())
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, tag.c_str(), sizeof(buffer) - 1);
        if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
            tag = std::string(buffer);
    }

    // Add Component button
    ImGui::SameLine();
    if (ImGui::Button("Add Component"))
        ImGui::OpenPopup("AddComponentPopup");
    DrawAddComponentPopup(entity);

    // Transform
    DrawComponent<TransformComponent>("Transform", entity, [](Entity, TransformComponent& tc)
    {
        DrawVec3Control("Translation", tc.Translation);
        DrawVec3Control("Rotation", tc.Rotation);
        DrawVec3Control("Scale", tc.Scale, 1.0f);
    }, /*removable*/false);

    // Camera
    DrawComponent<CameraComponent>("Camera", entity, [](Entity, CameraComponent& cc)
    {
        ImGui::Checkbox("Primary", &cc.Primary);
        ImGui::Checkbox("Fixed Aspect", &cc.FixedAspectRatio);
        // Add camera lens fields if you expose them
    });

    // Box collider
    DrawComponent<BoxShapeComponent>("Box Collider", entity, [](Entity, BoxShapeComponent& bc)
    {
        bool changed = false;
        changed |= DrawVec3Control("Half Extents", bc.HalfExtents);
        if (ImGui::Button("Rebuild")) { bc.Dirty = true; changed = true; }
        if (changed) bc.Dirty = true;
    });

    // Sphere collider
    DrawComponent<SphereShapeComponent>("Sphere Collider", entity, [](Entity, SphereShapeComponent& sc)
    {
        bool changed = false;
        changed |= ImGui::DragFloat("Radius", &sc.Radius, 0.01f, 0.0f, 1000.0f);
        if (ImGui::Button("Rebuild")) { sc.Dirty = true; changed = true; }
        if (changed) sc.Dirty = true;
    });

    // Rigidbody (mutually exclusive with Character)
    DrawComponent<RigidbodyComponent>("Rigidbody", entity, [](Entity e, RigidbodyComponent& rb)
    {
        const char* types[] = { "Static", "Dynamic", "Kinematic" };
        int current = (int)rb.Type;
        if (ImGui::Combo("Type", &current, types, IM_ARRAYSIZE(types)))
            rb.Type = (BodyType)current;

        ImGui::DragFloat("Mass", &rb.Mass, 0.05f, 0.0f, 100000.0f);
        ImGui::DragFloat("Linear Damping", &rb.LinearDamp, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Angular Damping", &rb.AngularDamp, 0.01f, 0.0f, 10.0f);
        // ImGui::Checkbox("Continuous (CCD)", &rb.EnableCCD);
        // ImGui::DragFloat("Friction", &rb.Friction, 0.01f, 0.0f, 1.0f);
        // ImGui::DragFloat("Restitution", &rb.Restitution, 0.01f, 0.0f, 1.0f);

        if (e.HasComponent<CharacterComponent>())
        {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1,0.6f,0.3f,1), "Warning: Character & Rigidbody on same entity.");
            if (ImGui::Button("Remove Character")) e.RemoveComponent<CharacterComponent>();
        }
    });

    // Character (mutually exclusive with Rigidbody)
    DrawComponent<CharacterComponent>("Character Controller", entity,
                                      [](Entity e, CharacterComponent &cc) {
                                        
					bool changed = false;
        changed |= ImGui::DragFloat("Half Height", &cc.HalfHeight, 0.01f, 0.0f, 1000.0f);
        changed |= ImGui::DragFloat("Radius", &cc.Radius, 0.01f, 0.0f, 1000.0f);
        changed |= ImGui::Checkbox("Continuous (CCD)", &cc.EnableCCD);
        changed |= ImGui::DragFloat("Gravity", &cc.Gravity, 0.05f, -100.0f, 100.0f);
        changed |= ImGui::DragFloat("Jump Impulse", &cc.JumpImpulse, 0.05f, 0.0f, 100.0f);
	changed |= ImGui::DragFloat("Capsule Offset Y", &cc.CapsuleOffsetY,0.0f, -100.0f, 100.0f);

        ImGui::Separator();
        ImGui::Text("Runtime (read-only)");
        ImGui::BeginDisabled();
        ImGui::Checkbox("Grounded", &cc.Grounded);
        ImGui::DragFloat("VerticalVel", &cc.VerticalVel, 0.01f);
        ImGui::EndDisabled();

        if (ImGui::Button("Rebuild Shape")) {
          changed = true;
        }
        if (changed) {
	  cc.Dirty = true;
	}

        if (e.HasComponent<RigidbodyComponent>())
        {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1,0.6f,0.3f,1), "Warning: Character & Rigidbody on same entity.");
            if (ImGui::Button("Remove Rigidbody")) e.RemoveComponent<RigidbodyComponent>();
        }
    });

    // Script
    DrawComponent<NativeScriptComponent>("Native Script", entity, [](Entity e, NativeScriptComponent& nsc)
    {
        // Minimal UI: show bound type if you store it, allow unbind
        if (nsc.Instance)
            ImGui::Text("Bound runtime instance: %p", (void*)nsc.Instance);
        else
            ImGui::Text("No instance (bind at runtime)");
        if (ImGui::Button("Remove Script"))
            e.RemoveComponent<NativeScriptComponent>();
    });
}

} // namespace UE

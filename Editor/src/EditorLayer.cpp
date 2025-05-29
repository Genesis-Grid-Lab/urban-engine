#include "EditorLayer.h"
#include <ImGuiFileDialog.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

EditorLayer::EditorLayer(const glm::vec2& size): m_Size(size) {}

void EditorLayer::OnAttach(){
    console.AddLog("Starting");
    m_IconPlay = Texture2D::Create("Resources/Icons/PlayButton.png");
    m_IconStop = Texture2D::Create("Resources/Icons/StopButton.png");
    m_ActiveScene = CreateRef<Scene>(m_Size.x, m_Size.y);
    m_SceneHierarchyPanel = SceneHierarchyPanel(m_ActiveScene);
    m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 2000.0f);

    auto commandLineArgs = Application::Get().GetCommandLineArgs();
    if(commandLineArgs.Count > 1){

        auto sceneFilePath = commandLineArgs[1];
        SceneSerializer serializer(m_ActiveScene);
        serializer.Deserialize(sceneFilePath);
    }

    console.AddLog("Loading Resources");
    // Ref<Model> castle = CreateRef<Model>("Resources/sponza/sponza.obj");   
    // Ref<Model> castle = CreateRef<Model>("Resources/sponza2/source/glTF/Sponza.gltf");   
    Ref<Model> sphere = CreateRef<Model>("Resources/sphere.fbx");
    Ref<Model> cube = CreateRef<Model>("Resources/cube.fbx");
    Ref<Model> Man = CreateRef<Model>("Resources/Animations/Idle.fbx");
    Ref<Animation> ManAnim = CreateRef<Animation>("Resources/Animations/Idle.fbx", Man);
    Ref<Animation> runAnim = CreateRef<Animation>("Resources/Animations/Running.fbx", Man);
    Ref<Animation> jumpAnim = CreateRef<Animation>("Resources/Animations/jump.fbx", Man);

    console.AddLog("Creating entity");
    auto& camEntt = m_ActiveScene->CreateEntity("Cam");
    auto& sceneCam = camEntt.AddComponent<CameraComponent>();
    auto& camTC = camEntt.GetComponent<TransformComponent>();
    camTC.Translation = {0.0f, 2.7f, 5.5f};  
    // camTC.Rotation = {0,0,0};

    class CameraController : public ScriptableEntity
    {
    public:
        virtual void OnCreate() override
        {

        }

        virtual void OnDestroy() override
        {
        }

        virtual void OnUpdate(Timestep ts) override
        {
            auto& translation = GetComponent<TransformComponent>().Translation;

            float speed = 5.0f;

        }
    };

    camEntt.AddComponent<NativeScriptComponent>().Bind<CameraController>();
    UE_CORE_WARN("DONE CAM");
    glm::vec3 cubePos = {0.0f, 0.0f, 0.0f};
    glm::vec3 cubeSize = {50, 1, 50};
    auto& floorEntity = m_ActiveScene->CreateEntity("Floor");
    floorEntity.AddComponent<CubeComponent>().Color = {0.5f, 0.0f, 0.5f};
    auto& FloorTC = floorEntity.GetComponent<TransformComponent>();
    auto& fbox = floorEntity.AddComponent<BoxShapeComponent>();
    FloorTC.Translation = cubePos;
    FloorTC.Scale = cubeSize; 

    auto& frb = floorEntity.AddComponent<RigidbodyComponent>();
    frb.Shape = fbox.Shape;
    frb.Type = JPH::EMotionType::Static;
    frb.Layer = Layers::NON_MOVING;
    frb.Activate = false;

    auto& sphereEntt = m_ActiveScene->CreateEntity("Sphere");
    sphereEntt.AddComponent<ModelComponent>().ModelData = sphere;
    auto& stc = sphereEntt.GetComponent<TransformComponent>();
    auto& sbox = sphereEntt.AddComponent<SphereShapeComponent>();   

    stc.Translation = {0,2,0};
    auto& srb = sphereEntt.AddComponent<RigidbodyComponent>();
    srb.Shape = sbox.Shape;
    srb.Type = JPH::EMotionType::Dynamic;
    srb.Layer = Layers::MOVING;    
    srb.Activate = true;    

    auto& cubeEntt = m_ActiveScene->CreateEntity("Cube");
    cubeEntt.AddComponent<ModelComponent>().ModelData = cube;
    auto& ctc = cubeEntt.GetComponent<TransformComponent>();
    ctc.Translation = {-1.0f, 2.0f, 2.0f};
    cubeEntt.AddComponent<BoxShapeComponent>();
    auto& crb = cubeEntt.AddComponent<RigidbodyComponent>();
    crb.Layer = Layers::MOVING;
    crb.Type = JPH::EMotionType::Dynamic;
    crb.Activate = true;    

    auto& manEntt = m_ActiveScene->CreateEntity("Man");
    auto& manTC = manEntt.GetComponent<TransformComponent>();
    manTC.Translation = {1, 2, 0};
    manTC.Scale = glm::vec3(0.1f);
    manTC.Rotation = {0, glm::radians(180.0f), 0};
    auto& manModel = manEntt.AddComponent<ModelComponent>();
    manModel.ModelData = Man;
    manModel.AnimationData["idle"] = ManAnim;
    manModel.AnimationData["run"] = runAnim;
    manModel.AnimationData["jump"] = jumpAnim;
    manEntt.AddComponent<BoxShapeComponent>();
    auto& manRB = manEntt.AddComponent<CharacterComponent>();
    // auto& manRB = manEntt.AddComponent<RigidbodyComponent>();
    manRB.Layer = Layers::MOVING;
    manRB.Type = JPH::EMotionType::Dynamic;
    manRB.Activate = true;    

    class PlayerController : public ScriptableEntity
    {
    public:
        virtual void OnCreate() override
        {
            m_Scene = GetScene();
            auto& translation = GetComponent<TransformComponent>().Translation;
            translation.x = rand() % 10 - 5.0f;

            auto view = m_Scene->GetRegistry().view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
				
				if (camera.Primary)
				{
					Cam = Entity(entity, m_Scene);
					break;
				}
			}
        }

        virtual void OnDestroy() override
        {
        }

        virtual void OnUpdate(Timestep ts) override
        {
            auto& rigid = GetComponent<CharacterComponent>();
            // auto& rigid = GetComponent<RigidbodyComponent>();
            auto& translation = GetComponent<TransformComponent>().Translation;
            auto& rotation = GetComponent<TransformComponent>().Rotation;
            auto& camTC = Cam.GetComponent<TransformComponent>().Translation;  
            auto& camRot = Cam.GetComponent<TransformComponent>().Rotation;
            auto& model = GetComponent<ModelComponent>().ModelData;
            auto& modelAnim = GetComponent<ModelComponent>().AnimationData;
            auto& camCam = Cam.GetComponent<CameraComponent>().Camera;
           
            float speed = 10.0f;

            bool running = false, idle = true, jump = false;

            glm::vec3 moveDir = glm::vec3(0.0f);       
            glm::vec3 camTarget = translation + glm::vec3(0, 1, 0);
            
            if (Input::IsMouseButtonPressed(Mouse::ButtonRight)) {
                glm::vec2 mouseDelta = Input::GetMouseDelta(); // You need to implement or wrap this
                UE_CORE_INFO("Delta {}", mouseDelta);
                m_Yaw += mouseDelta.x * m_MouseSensitivity;
                m_Pitch += mouseDelta.y * m_MouseSensitivity;                
                m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
            }

            glm::vec3 camDir;
            camDir.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
            camDir.y = sin(glm::radians(m_Pitch));
            camDir.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
            camDir = glm::normalize(camDir);
            
            glm::vec3 camPos = camTarget - camDir * m_Distance;

            camTC = camPos;

            glm::vec3 forward = glm::normalize(glm::vec3(camDir.x, 0, camDir.z));
            glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

            if (Input::IsKeyPressed(Key::W)) moveDir -= forward; running = true;
            if (Input::IsKeyPressed(Key::S)) moveDir += forward; running = true;
            if (Input::IsKeyPressed(Key::A)) moveDir += right; running = true;
            if (Input::IsKeyPressed(Key::D)) moveDir -= right; running = true;
            if (Input::IsKeyPressed(Key::Space)) moveDir.y = 1; jump = true;
            // float rotSpeed = 2.5f;
            // if (Input::IsKeyPressed(Key::A)) rotation.y -= rotSpeed * ts;
            // if (Input::IsKeyPressed(Key::D)) rotation.y += rotSpeed * ts;

            if (glm::length(moveDir) > 0.0f) {
                moveDir = glm::normalize(moveDir);
                rigid.Velocity = moveDir * speed;

                float targetYaw = glm::degrees(atan2(moveDir.x, moveDir.z));
                rotation.y = glm::radians(targetYaw); // Or + depending on orientation
            } else {
                rigid.Velocity = glm::vec3(0.0f);
                idle = true;
                running = false;
                jump = false;
            }

            
            if(running){
                idle = false;
                jump = false;
                Renderer3D::RunAnimation(modelAnim["run"], ts);
            }

            if(jump){
                running = false;
                idle = false;
                Renderer3D::RunAnimation(modelAnim["jump"], ts);
            }

            if(idle){
                Renderer3D::RunAnimation(modelAnim["idle"], ts);
            }


   
            rigid.Character->SetLinearVelocity(JPH::Vec3(rigid.Velocity.x, rigid.Velocity.y, rigid.Velocity.z));            
            // GetBodyInterface().SetLinearVelocity(rigid.ID,JPH::Vec3(rigid.Velocity.x, rigid.Velocity.y, rigid.Velocity.z));

            // rigid.Character->GetGroundNormal().SetY(0.0f);

            glm::vec3 camOffset = glm::vec3(
                -glm::sin(rotation.y) * 5.0f,
                2.0f,
                -glm::cos(rotation.y) * 5.0f
            );

            camCam.SetTarget(camTarget);
            camCam.SetMode(m_Mode);
            // camCam.SetOffset(camOffset);
            camCam.SetOffset(camDir * 5.0f + glm::vec3(0, 2, 0)); // Third-person view

           
        }

        virtual void OnImGuiRender() override{
            ImGui::Text("TESTING");
        }

    private:
        Scene* m_Scene;
        Entity Cam;
        CameraMode m_Mode = CameraMode::ThirdPerson;
        float m_Yaw = 0.0f;
        float m_Pitch = 0.0f;
        float m_MouseSensitivity = 0.1f;
        float m_Distance = 5.0f; // Camera distance behind player
    };

    manEntt.AddComponent<NativeScriptComponent>().Bind<PlayerController>();


    // auto& castleEntt = m_ActiveScene->CreateEntity("Castle");
    // castleEntt.AddComponent<ModelComponent>().ModelData = castle;
    // auto& casttc = castleEntt.GetComponent<TransformComponent>();
    // casttc.Translation = {0,0,0};
    // casttc.Scale = glm::vec3(0.5f);
    // auto& castbox = castleEntt.AddComponent<BoxShapeComponent>();    
    // auto& castrb = castleEntt.AddComponent<RigidbodyComponent>();
    // castrb.Shape = castbox.Shape;
    // castrb.Type = JPH::EMotionType::Static;
    // castrb.Layer = Layers::NON_MOVING;
    // castrb.Activate = false;

    console.AddLog("Done Creating entity");
    UE_CORE_INFO("Done Creating entity");
}

void EditorLayer::OnUpdate(Timestep ts){
    // Resize
    if (FramebufferSpecification spec = m_ActiveScene->m_Framebuffer->GetSpecification();
        m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
        (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
    {        
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
    }    
    
    auto[mx, my] = ImGui::GetMousePos();
    mx -= m_ViewportBounds[0].x;
    my -= m_ViewportBounds[0].y;
    glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
    my = viewportSize.y - my;
    int mouseX = (int)mx;
    int mouseY = (int)my;

    switch (m_SceneState)
    {
        case SceneState::Edit:
        {
            // m_ActiveScene->OnUpdateRuntime(ts, mouseX, mouseY, viewportSize);
            m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera, mouseX, mouseY, viewportSize);
            m_EditorCamera.OnUpdate(ts);            
            if(m_ViewportFocused && m_ViewportHovered)
                m_ActiveScene->OnMouseInput(Input::GetMouseX(), Input::GetMouseY(), Input::IsMouseButtonPressed(0), ts);
            break;
        }
        case SceneState::Play:
        {
            m_ActiveScene->OnUpdateRuntime(ts, mouseX, mouseY, viewportSize);
            break;
        }
    }

        
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent& e){
    // Shortcuts
    if (e.GetRepeatCount() > 0)
        return false;

    bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
    bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

    switch (e.GetKeyCode())
    {
        case Key::N:
        {
            // if (control)
            //     NewScene();

            break;
        }
        case Key::O:
        {
            // if (control)
            //     OpenScene();

            break;
        }
        case Key::S:
        {
            if (control)
            {
                // if (shift)
                //     SaveSceneAs();
                // else
                //     SaveScene();
            }

            break;
        }

        // Scene Commands
        case Key::D:
        {
            // if (control)
            //     OnDuplicateEntity();

            break;
        }

        // Gizmos
        case Key::Q:
        {
            if (!ImGuizmo::IsUsing())
                m_GizmoType = -1;
            break;
        }
        case Key::W:
        {            
            if (!ImGuizmo::IsUsing())
                m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
            break;
        }
        case Key::E:
        {
            if (!ImGuizmo::IsUsing())
                m_GizmoType = ImGuizmo::OPERATION::ROTATE;
            break;
        }
        case Key::R:
        {
            if (!ImGuizmo::IsUsing())
                m_GizmoType = ImGuizmo::OPERATION::SCALE;
            break;
        }
    }
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
{
    if (e.GetMouseButton() == Mouse::ButtonLeft)
    {
        if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
            m_SceneHierarchyPanel.SetSelectedEntity(m_ActiveScene->GetHoveredEntity());
    } 
    return false;
}
void EditorLayer::OnEvent(Event& e){
    m_EditorCamera.OnEvent(e);
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
}

void EditorLayer::OnImGuiRender(){
    // Note: Switch this to true to enable dockspace
    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    // float minWinSizeX = style.WindowMinSize.x;
    // style.WindowMinSize.x = 370.0f;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    // style.WindowMinSize.x = minWinSizeX;

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows, 
            // which we can't undo at the moment without finer window depth/z control.
            //ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);1
            if (ImGui::MenuItem("New", "Ctrl+N"))
            {
                NewScene();
                console.AddLog("Open New Scene");
            }

            if (ImGui::MenuItem("Open...", "Ctrl+O"))
            {
                OpenScene();
                console.AddLog("Open Scene");
            }

            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
            {
                SaveScene();
                console.AddLog("Save Scene");
            }                

            if (ImGui::MenuItem("Exit")){
                console.AddLog("Good bye");
                Application::Get().Close();
            }

            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("View")){

            if(ImGui::MenuItem("Show Boxes"))
            {
                if(m_ActiveScene->ShowBoxes){
                    m_ActiveScene->ShowBoxes = false;
                    console.AddLog("Show Boxes false");
                }
                else{
                    m_ActiveScene->ShowBoxes = true;
                    console.AddLog("Show Boxes true");
                }
            }

            if(ImGui::MenuItem("Show Cams")){
                if(m_ActiveScene->ShowCams){
                    m_ActiveScene->ShowCams = false;
                    console.AddLog("Show Cam false");
                }
                else{
                    m_ActiveScene->ShowCams = true;
                    console.AddLog("Show Cam true");
                }
            }

            if(ImGui::MenuItem("Show Boxes Play")){
                if(m_ActiveScene->ShowBoxesPlay){
                    m_ActiveScene->ShowBoxesPlay = false;
                    console.AddLog("Show Boxes play false");
                }
                else{
                    m_ActiveScene->ShowBoxesPlay = true;
                    console.AddLog("Show Boxes play true");
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    m_SceneHierarchyPanel.OnImGuiRender();
    m_ContentBrowserPanel.OnImGuiRender();
    bool p_open = true;
    console.Draw("Console", &p_open);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
    ImGui::Begin("Viewport");
    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    auto viewportOffset = ImGui::GetWindowPos();
    m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
    m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();
    Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

    ImTextureID textureID = m_ActiveScene->m_Framebuffer->GetColorAttachmentRendererID();
    ImGui::Image(textureID, ImVec2{m_ViewportSize.x, m_ViewportSize.y}, ImVec2{0, 1}, ImVec2{1,0});

    //dragging    

    // Gizmos
    Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
    if (selectedEntity && m_GizmoType != -1)
    {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();        

        ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

        // Camera
        
        // Runtime camera from entity
        // auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
        // const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
        // const glm::mat4& cameraProjection = camera.GetProjection();
        // glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

        // Editor camera
        const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
        glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

        // Entity transform
        auto& tc = selectedEntity.GetComponent<TransformComponent>();
        glm::mat4 transform = tc.GetTransform();
        // ImGuizmo::DrawCubes(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), glm::value_ptr(transform), 1);

        // Snapping
        bool snap = Input::IsKeyPressed(Key::LeftControl);
        float snapValue = 0.5f; // Snap to 0.5m for translation/scale
        // Snap to 45 degrees for rotation
        if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
            snapValue = 45.0f;

        float snapValues[3] = { snapValue, snapValue, snapValue };

        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
            (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
            nullptr, snap ? snapValues : nullptr);

        if (ImGuizmo::IsUsing())
        {            
            glm::vec3 translation, rotation, scale;
            Math::DecomposeTransform(transform, translation, rotation, scale);

            glm::vec3 deltaRotation = rotation - tc.Rotation;
            tc.Translation = translation;
            tc.Rotation += deltaRotation;
            tc.Scale = scale;
        }
        
    }

    ImGui::End();
    ImGui::PopStyleVar();
    
    if (ImGuiFileDialog::Instance()->Display("OpenScene")) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            // action
            if(!filePathName.empty())
                OpenScene(filePathName);
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("SaveSceneAs")) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::string SavefilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string SavefilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            // action
            if (!SavefilePathName.empty())
            {
                SerializeScene(m_ActiveScene, SavefilePathName);
                m_EditorScenePath = SavefilePathName; 
            }
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }

    UI_Toolbar();

    ImGui::End();
}

void EditorLayer::UI_Toolbar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    auto& colors = ImGui::GetStyle().Colors;
    const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
    const auto& buttonActive = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

    ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    float size = ImGui::GetWindowHeight() - 4.0f;
    Ref<Texture2D> icon = m_SceneState == SceneState::Edit ? m_IconPlay : m_IconStop;
    ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
    if (ImGui::ImageButton("##play",(ImTextureID)icon->GetRendererID(), ImVec2(size, size)))
    {
        if (m_SceneState == SceneState::Edit)
            OnScenePlay();
        else if (m_SceneState == SceneState::Play)
            OnSceneStop();
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
    ImGui::End();
}

void EditorLayer::NewScene()
{
    
    m_ActiveScene = CreateRef<Scene>(m_Size.x, m_Size.y);
    m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
    m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    
    m_EditorScenePath = std::filesystem::path();
}

void EditorLayer::OpenScene()
{
    // std::string filepath = FileDialogs::OpenFile("Urban Engine Scene (*.UE)\0*.UE\0");    
    // if (!filepath.empty())
    //     OpenScene(filepath);
    IGFD::FileDialogConfig config;
    config.path = ".";
    ImGuiFileDialog::Instance()->OpenDialog("OpenScene", "Choose File", ".UE", config);  
}

void EditorLayer::OpenScene(const std::filesystem::path& path)
{
    if (m_SceneState != SceneState::Edit)
        OnSceneStop();

    if (path.extension().string() != ".UE")
    {
        UE_WARN("Could not load {0} - not a scene file", path.filename().string());
        return;
    }
    
    Ref<Scene> newScene = CreateRef<Scene>(m_Size.x, m_Size.y);
    SceneSerializer serializer(newScene);
    if (serializer.Deserialize(path.string()))
    {
        m_EditorScene = newScene;
        m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_SceneHierarchyPanel.SetContext(m_EditorScene);

        m_ActiveScene = m_EditorScene;
        m_EditorScenePath = path;
    }
}

void EditorLayer::SaveScene()
{
    if (!m_EditorScenePath.empty())
        SerializeScene(m_ActiveScene, m_EditorScenePath);
    else
        SaveSceneAs();
}

void EditorLayer::SaveSceneAs()
{
    // std::string filepath = FileDialogs::SaveFile("Urban Engine Scene (*.UE)\0*.UE\0");    
    // if (!filepath.empty())
    // {
    //     SerializeScene(m_ActiveScene, filepath);
    //     m_EditorScenePath = filepath;
    // }

    IGFD::FileDialogConfig config;
    config.path = ".";
    ImGuiFileDialog::Instance()->OpenDialog("SaveSceneAs", "Choose File", ".UE", config); 
}

void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
{
    SceneSerializer serializer(scene);
    serializer.Serialize(path.string());
}

void EditorLayer::OnScenePlay()
{
    m_SceneState = SceneState::Play;

    m_EditorScene = Scene::Copy(m_ActiveScene);
    m_ActiveScene->OnRuntimeStart();

    m_SceneHierarchyPanel.SetContext(m_ActiveScene);
}

void EditorLayer::OnSceneStop()
{
    m_SceneState = SceneState::Edit;

    m_ActiveScene->OnRuntimeStop();
    m_ActiveScene = m_EditorScene;

    m_SceneHierarchyPanel.SetContext(m_ActiveScene);
}

void EditorLayer::OnDuplicateEntity()
{
    if (m_SceneState != SceneState::Edit)
        return;

    Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
    if (selectedEntity)
        m_EditorScene->DuplicateEntity(selectedEntity);
}
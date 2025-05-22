#include "uepch.h"
#include "Scene/Scene.h"
#include "Core/UE_Assert.h"
#include "Scene/Entity.h"

namespace UE {
    Scene::Scene() {
        camera.position = Vector3{ 10.0f, 10.0f, 10.0f }; // Camera position
        camera.target = Vector3{ 0.0f, 0.0f, 0.0f };      // Camera looking at point
        camera.up = Vector3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
        camera.fovy = 45.0f;                                // Camera field-of-view Y
        camera.projection = CAMERA_PERSPECTIVE;

		m_Buffer = CreateScope<Framebuffer>(GetScreenWidth(), GetScreenHeight());
	}

    Scene::Scene(uint32_t width, uint32_t height){
		// m_ViewportWidth = width;
		// m_ViewportHeight = height;

		// FramebufferSpecification fbSpec;
        // fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        // fbSpec.Width = width;
        // fbSpec.Height = height;
        // m_Framebuffer = Framebuffer::Create(fbSpec);

		// m_Physics3D.Init();
	}
    Scene::~Scene(){
		// m_Physics3D.CleanUp();
		ViewEntity<Entity, SkyboxComponent>([this](auto entiy, auto& comp){
			UnloadSkybox(&comp.Skybox);			
		});
	}

    template<typename Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto view = src.view<Component>();
		for (auto e : view)
		{
			UUID uuid = src.get<IDComponent>(e).ID;
			UE_CORE_ASSERT(enttMap.find(uuid) != enttMap.end());
			entt::entity dstEnttID = enttMap.at(uuid);

			auto& component = src.get<Component>(e);
			dst.emplace_or_replace<Component>(dstEnttID, component);
		}
	}

    template<typename Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		if (src.HasComponent<Component>())
			dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
	}

    Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>(other->m_ViewportWidth, other->m_ViewportHeight);

		newScene->ShowBoxes = other->ShowBoxes;
		newScene->ShowBoxesPlay = other->ShowBoxesPlay;
		newScene->ShowCams = other->ShowCams;
		// newScene->m_Physics3D. = other->m_Physics3D;
		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// Copy components (except IDComponent and TagComponent)
		CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		// CopyComponent<PlaneComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		// CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);		
		// CopyComponent<ModelComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);		
		// CopyComponent<CubeComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);		
		// CopyComponent<RigidbodyComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);		
		// CopyComponent<BoxShapeComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);		
		// CopyComponent<SphereShapeComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);	
		// CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);	
		// CopyComponent<CharacterComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);	

		return newScene;
	}

    Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

    void Scene::OnRuntimeStart(){
    }

    void Scene::OnRuntimeStop()
	{
		
	}

    void Scene::OnUpdateRuntime(Timestep ts){
    }

    void Scene::OnUpdateEditor(Timestep ts){		
		m_Buffer->Bind();
        ClearBackground(RAYWHITE);

        UpdateCamera(&camera, CAMERA_FREE);
         if (IsKeyPressed('Z')) camera.target = Vector3{ 0.0f, 0.0f, 0.0f };

        BeginMode3D(camera);  
		
		ViewEntity<Entity, SkyboxComponent>([this](auto entiy, auto& comp){
			SkyboxUpdate(&comp.Skybox);
			DrawSkyboxModel(&comp.Skybox);
		});

        ViewEntity<Entity, PlaneComponent>([this] (auto entity, auto& comp){

			auto& transform = entity.template GetComponent<TransformComponent>();	
			DrawPlane(transform.Translation, comp.Size, comp.color);	
		});	

		ViewEntity<Entity, ModelComponent>([this] (auto entity, auto& comp){

			auto& transform = entity.template GetComponent<TransformComponent>();	
			// DrawModel(comp.mModel,transform.Translation , 1.0f, WHITE);
			// BoundingBox box = GetModelBoundingBox(comp.mModel);
			// DrawBoundingBox(box, RED);
			DrawMesh(comp.mModel.meshes[0], LoadMaterialDefault(), MatrixIdentity());
		});	
		// DrawCube(Vector3Zero(), 1, 1, 1, RED);

        EndMode3D();
		ViewEntity<Entity, SkyboxComponent>([this](auto entiy, auto& comp){
			DrawSkyboxTexture(&comp.Skybox);			
		});
		m_Buffer->Unbind();
    }

    void Scene::DuplicateEntity(Entity entity)
	{
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);

		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		// CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
		// CopyComponentIfExists<CameraComponent>(newEntity, entity);		
	}

    template<typename T>
	void  Scene::OnComponentAdded(Entity entity, T& component)
	{
		// static_assert(false);
	}

	template<>
	void  Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	}

	template<>
	void  Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void  Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

    template<>
	void  Scene::OnComponentAdded<PlaneComponent>(Entity entity, PlaneComponent& component)
	{
	}

	template<>
	void  Scene::OnComponentAdded<SkyboxComponent>(Entity entity, SkyboxComponent& component)
	{
	}
	template<>
	void  Scene::OnComponentAdded<ModelComponent>(Entity entity, ModelComponent& component)
	{
	}
	
}
#include "uepch.h"
#include "Scene/Scene.h"
#include "Scene/SceneCamera.h"
#include "Scene/Components.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Renderer3D.h"
#include "Renderer/RenderCommand.h"
#include <glm/glm.hpp>
#include "UE_Assert.h"
#include "Scene/Entity.h"
#include "Scene/ScriptableEntity.h"
#include "Core/Log.h"
#include <Jolt/Math/Math.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
//temp
#include <glad/glad.h>


namespace UE {

	Entity GlobHovered;

	Scene::Scene() {

	}

    Scene::Scene(uint32_t width, uint32_t height){
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = width;
        fbSpec.Height = height;
        m_Framebuffer = Framebuffer::Create(fbSpec);

		m_Physics3D.Init();
	}
    Scene::~Scene(){
		m_Physics3D.CleanUp();
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
		CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);		
		CopyComponent<ModelComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);		
		CopyComponent<CubeComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);		
		CopyComponent<RigidbodyComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);		
		CopyComponent<BoxShapeComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);		
		CopyComponent<SphereShapeComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);	
		CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);	
		CopyComponent<CharacterComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);	

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

	void Scene::ReadPixelEntity(int& mouseX, int& mouseY, glm::vec2& viewportSize){
		if(mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y){            
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			// UE_INFO("mx: {0}, my: {1}" ,Input::GetMouseX(), Input::GetMouseY() );
			if(pixelData < -1 || pixelData >= 1036831949)
				pixelData = -1;
			UE_INFO("Pixel {0}",pixelData);
			GlobHovered = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, this);            
		}
	}

	void Scene::OnRuntimeStart(){
		JPH::BodyInterface &body_interface = m_Physics3D._physics_system->GetBodyInterface();

		ViewEntity<Entity, BoxShapeComponent>([this] (auto entity, auto& comp){
			glm::vec3 minBounds(FLT_MAX);
			glm::vec3 maxBounds(-FLT_MAX);
			glm::vec3 center = glm::vec3(0);
			glm::vec3 halfExtents = glm::vec3(0);
			glm::vec3 scaledHalfExtents = glm::vec3(0);
			auto& transform = entity.template GetComponent<TransformComponent>();

			if(entity.HasComponent<ModelComponent>()){
				auto& modelComp = entity.GetComponent<ModelComponent>();

				for(auto& mesh : modelComp.ModelData->GetMeshes()){
					for(const auto& vertex : mesh->m_Vertices){
						minBounds = glm::min(minBounds, vertex.Position);
        				maxBounds = glm::max(maxBounds, vertex.Position);
					}
				}
			}

			if(entity.HasComponent<CubeComponent>()){
				auto& cubeComp = entity.GetComponent<CubeComponent>();

				for(const auto& vertex : Renderer3D::GetCubeMesh()->m_Vertices){
					minBounds = glm::min(minBounds, vertex.Position);
					maxBounds = glm::max(maxBounds, vertex.Position);
				}
			}

			center = (minBounds + maxBounds) * 0.5f;
			halfExtents = (maxBounds - minBounds) * 0.5f;

			// Apply scale if needed
			scaledHalfExtents = halfExtents * transform.Scale;

			if(!comp.Settings){
				comp.Settings =  new JPH::BoxShapeSettings{
					{ scaledHalfExtents.x, scaledHalfExtents.y, scaledHalfExtents.z }
				};
			}	

			JPH::ShapeSettings::ShapeResult shape_result = comp.Settings->Create();

			comp.Shape =  new JPH::BoxShape(*comp.Settings, shape_result);  

			if (shape_result.HasError())
			{
				UE_CORE_WARN("Error creating shape: {}",
							shape_result.GetError());
			}
			
			auto& box = comp.Shape->GetLocalBounds();	

		});	

		ViewEntity<Entity, SphereShapeComponent>([this](auto entity, auto& comp){
			auto& transform = entity.template GetComponent<TransformComponent>();

			if(!comp.Settings){
				comp.Settings = new JPH::SphereShapeSettings{
					transform.GetRadius()
				};
			}

			JPH::ShapeSettings::ShapeResult shape_result = comp.Settings->Create();

			comp.Shape =  new JPH::SphereShape(*comp.Settings, shape_result);  

			if (shape_result.HasError())
			{
				UE_CORE_WARN("Error creating shape: {}",
							shape_result.GetError());
			}	
		});

		ViewEntity<Entity, RigidbodyComponent>([this, &body_interface](auto entity, auto& comp){
			auto& transform = entity.template GetComponent<TransformComponent>();

			// Create the settings for the body itself. Note that here you can also set
			// other properties like the restitution / friction.	
			if(entity.HasComponent<BoxShapeComponent>()){
				auto& boxComp = entity.GetComponent<BoxShapeComponent>();
				comp.Shape = boxComp.Shape;
			}
			else if(entity.HasComponent<SphereShapeComponent>()){
				auto& boxComp = entity.GetComponent<SphereShapeComponent>();
				comp.Shape = boxComp.Shape;
			}
			else{
				UE_CORE_WARN("No Shape in RgidBodyComponent");
				return;
			}

			if(!comp.Setting){
				comp.Setting = new JPH::BodyCreationSettings(
					comp.Shape,
					//JPH::RVec3(0.0_r, -1.0_r, 0.0_r),
					JPH::RVec3(transform.Translation.x, transform.Translation.y, transform.Translation.z),
					JPH::Quat::sIdentity(),
					comp.Type,
					comp.Layer);			
			}


			// Create the actual rigid body
			if(!comp.Body){
				comp.Body = body_interface.CreateBody(
					*comp.Setting); // Note that if we run out of bodies this can return
									// nullptr
			}

			if (comp.Body == nullptr)
			{
				UE_CORE_WARN("Error creating floor body interface. There might be too "
							"many bodies.");
			}

			// physComp.Body->SetFriction(0.5);

			// Add it to the world
			body_interface.AddBody(comp.Body->GetID(), (JPH::EActivation)comp.Activate);

			comp.ID = comp.Body->GetID();

			body_interface.ActivateBody(comp.ID);
		});

		ViewEntity<Entity, CharacterComponent>([this] (auto entity, auto& comp){
			glm::vec3 minBounds(FLT_MAX);
			glm::vec3 maxBounds(-FLT_MAX);
			glm::vec3 center = glm::vec3(0);
			glm::vec3 halfExtents = glm::vec3(0);
			glm::vec3 scaledHalfExtents = glm::vec3(0);
			auto& transform = entity.template GetComponent<TransformComponent>();

			if(entity.HasComponent<ModelComponent>()){
				auto& modelComp = entity.GetComponent<ModelComponent>();

				for(auto& mesh : modelComp.ModelData->GetMeshes()){
					for(const auto& vertex : mesh->m_Vertices){
						minBounds = glm::min(minBounds, vertex.Position);
        				maxBounds = glm::max(maxBounds, vertex.Position);
					}
				}
			}

			center = (minBounds + maxBounds) * 0.5f;
			halfExtents = (maxBounds - minBounds) * 0.5f;

			scaledHalfExtents = halfExtents * transform.Scale;
			glm::vec3 joltCenterOffset = { center.x * transform.Scale.x, center.y * transform.Scale.y, center.z * transform.Scale.z };
			JPH::Vec3 offset = {joltCenterOffset.x, joltCenterOffset.y, joltCenterOffset.z};

			if(entity.HasComponent<BoxShapeComponent>()){
				auto& boxComp = m_Registry.get<BoxShapeComponent>(entity);
				boxComp.Shape = new JPH::BoxShape(
					{ 0.25f, scaledHalfExtents.y, scaledHalfExtents.z }
				);

				auto result = JPH::RotatedTranslatedShapeSettings(offset, JPH::Quat::sIdentity(), boxComp.Shape).Create();
				if (result.HasError()) {
					UE_CORE_ERROR("Failed to create offset shape: {}", result.GetError());
					return;
				}
				comp.Shape = result.Get();
				// comp.Shape = boxComp.Shape;
			}
			else{
				UE_CORE_WARN("No Shape in CharacterComponent");
				return;
			}

			if(!comp.Setting){
				comp.Setting = new JPH::CharacterSettings();
				comp.Setting->mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
				comp.Setting->mLayer = comp.Layer;
				comp.Setting->mShape = comp.Shape;
				comp.Setting->mFriction = 0.5f;
				comp.Setting->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), 0.3f); // Accept contacts that touch the lower sphere of the capsule								
			}

			if(!comp.Character){
				comp.Character = new JPH::Character(comp.Setting, 
					{transform.Translation.x, transform.Translation.y, transform.Translation.z}, JPH::Quat::sIdentity(), 0, m_Physics3D._physics_system);				
			}			

			comp.ID = comp.Character->GetBodyID();
			comp.Character->AddToPhysicsSystem((JPH::EActivation)comp.Activate);			

			comp.Character->Activate();
			// transform.Translation.y -= center.y * transform.Scale.y - scaledHalfExtents.y;

		});		

		m_Physics3D.StartSimulation();
	}

	void Scene::OnRuntimeStop()
	{
		// delete m_PhysicsWorld;
		// m_PhysicsWorld = nullptr;
	}

	void Scene::PhysicsUpdate(float dt){

		++m_Physics3D._step;		

		ViewEntity<Entity, RigidbodyComponent>([this, &dt] (auto entity, auto& comp){

			auto& transform = entity.template GetComponent<TransformComponent>();	
			const JPH::BodyInterface &body_interface{
				m_Physics3D._physics_system->GetBodyInterface()};
			if (!body_interface.IsActive(comp.ID))
			{				
				return;
			}

			const JPH::RVec3 position{
				body_interface.GetCenterOfMassPosition(comp.ID)};
			const JPH::Vec3 velocity{body_interface.GetLinearVelocity(comp.ID)};

			// UE_CORE_INFO("Step {}: Position = ({:.{}f}, {:.{}f}, "
			// 			"{:.{}f}), Velocity = ({:.{}f}, {:.{}f}, {:.{}f})\n",
			// 			m_Physics3D._step,
			// 			position.GetX(),
			// 			2,
			// 			position.GetY(),
			// 			2,
			// 			position.GetZ(),
			// 			2,
			// 			velocity.GetX(),
			// 			2,
			// 			velocity.GetY(),
			// 			2,
			// 			velocity.GetZ(),
			// 			2);

			transform.Translation =
				{position.GetX(), position.GetY(), position.GetZ()};
			// If you take larger steps than 1 / 60th of a second you need to do
			// multiple collision steps in order to keep the simulation stable. Do 1
			// collision step per 1 / 60th of a second (round up).
			constexpr int cCollisionSteps{1};

			// Step the world
			JPH::TempAllocatorImpl temp_allocator{10 * 1'024 * 1'024};
			m_Physics3D._physics_system->Update(dt,
									cCollisionSteps,
									&temp_allocator,
									m_Physics3D._job_system.get());
				
		});			
		
		ViewEntity<Entity, CharacterComponent>([this, &dt] (auto entity, auto& comp){
			auto& transform = entity.template GetComponent<TransformComponent>();

			const JPH::BodyInterface &body_interface{
				m_Physics3D._physics_system->GetBodyInterface()};
			if (!body_interface.IsActive(comp.ID))
			{		
				UE_CORE_WARN("Entity not active");
				return;
			}		
						

			// Output current position and velocity of the sphere
			const JPH::RVec3 position{
				body_interface.GetCenterOfMassPosition(comp.ID)};
			const JPH::Vec3 velocity{body_interface.GetLinearVelocity(comp.ID)};
			UE_CORE_INFO("Character Step {}: Position = ({:.{}f}, {:.{}f}, "
						"{:.{}f}), Velocity = ({:.{}f}, {:.{}f}, {:.{}f})\n",
						m_Physics3D._step,
						position.GetX(),
						2,
						position.GetY(),
						2,
						position.GetZ(),
						2,
						velocity.GetX(),
						2,
						velocity.GetY(),
						2,
						velocity.GetZ(),
						2);

			transform.Translation =
				{position.GetX(), position.GetY(), position.GetZ()};
			// If you take larger steps than 1 / 60th of a second you need to do
			// multiple collision steps in order to keep the simulation stable. Do 1
			// collision step per 1 / 60th of a second (round up).
			constexpr int cCollisionSteps{1};

			// Step the world
			JPH::TempAllocatorImpl temp_allocator{10 * 1'024 * 1'024};
			m_Physics3D._physics_system->Update(dt,
									cCollisionSteps,
									&temp_allocator,
									m_Physics3D._job_system.get());
		});
	}

    void Scene::OnUpdateRuntime(Timestep ts, int& mouseX, int& mouseY, glm::vec2& viewportSize){
		
		m_Framebuffer->Bind();
		// Clear our entity ID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);
		RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    	RenderCommand::Clear();

		// Update scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				JPH::BodyInterface &body_interface = m_Physics3D._physics_system->GetBodyInterface();
				// TODO: Move to Scene::OnScenePlay
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, this };
					nsc.Instance->m_Scene = this;
					nsc.Instance->m_BodyInterface = &body_interface;
					nsc.Instance->OnCreate();
				}

				nsc.Instance->OnUpdate(ts);
			});
		}
		
		PhysicsUpdate(ts);

		// Render 3D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		glm::vec3 pos;
		TransformComponent tc;
		{
			
			ViewEntity<Entity, CameraComponent>([this, &mainCamera, &pos, &tc] (auto entity, auto& comp){

				auto& transform = entity.template GetComponent<TransformComponent>();	
				comp.Camera.m_Position = &transform.Translation;
				// comp.Camera.m_Rotation2 = &transform.Rotation;
				if(comp.Primary){
					mainCamera = &comp.Camera;
					pos = transform.Translation;
					tc = transform;					
				}
				
			});
			
		}

		
		if (mainCamera)
		{			
			
			// Renderer3D::BeginCamera(*mainCamera, tc);
			Renderer3D::BeginCamera(*mainCamera);

			Renderer3D::RenderLight({5.5f, 5.0f, 0.3f });

			auto modelGroup = m_Registry.group<ModelComponent>(entt::get<TransformComponent>);
			for (auto entity : modelGroup) {
				auto [transform, modelComp] = modelGroup.get<TransformComponent, ModelComponent>(entity);				
				
				Renderer3D::DrawModel(modelComp.ModelData, transform.GetTransform(), glm::vec3(1.0f),(int)entity);			
			}

			auto CubeGroup = m_Registry.group<CubeComponent>(entt::get<TransformComponent>);
			for (auto entity : CubeGroup) {
				auto [transform, CubeComp] = CubeGroup.get<TransformComponent, CubeComponent>(entity);
				
				Renderer3D::DrawCube(transform.GetTransform(), CubeComp.Color, (int)entity);
			}

			auto boxShapeGroup = m_Registry.group<BoxShapeComponent>(entt::get<TransformComponent>);
			for (auto entity : boxShapeGroup) {
				auto [transform, boxComp] = boxShapeGroup.get<TransformComponent, BoxShapeComponent>(entity);				
				
				auto& box = boxComp.Shape->GetLocalBounds();

				if(ShowBoxesPlay)
					Renderer3D::DrawWireCube({transform.Translation.x, transform.Translation.y, transform.Translation.z}, 
					{box.GetSize().GetX(), box.GetSize().GetY(), box.GetSize().GetZ()}, 
					{0,1,0}, 0.5f);			
				
			}

			auto sphereShapeGroup = m_Registry.group<SphereShapeComponent>(entt::get<TransformComponent>);
			for (auto entity : sphereShapeGroup) {
				auto [transform, sphereComp] = sphereShapeGroup.get<TransformComponent, SphereShapeComponent>(entity);				
				
				auto& box = sphereComp.Shape->GetLocalBounds();

				if(ShowBoxesPlay)
					Renderer3D::DrawWireSphere({transform.Translation.x, transform.Translation.y, transform.Translation.z}, 
					{box.GetSize().GetX(), box.GetSize().GetY(), box.GetSize().GetZ()}, 
					{0,0,1}, 0.5f);			
				
			}
			

			Renderer3D::EndCamera();

			Renderer2D::BeginCamera(*mainCamera);
			ViewEntity<Entity, UIElement>([this] (auto entity, auto& comp){

				auto& transform = entity.template GetComponent<TransformComponent>();			
				Renderer2D::DrawUI(transform.GetTransform(), comp);
			});

			auto group1 = m_Registry.group<ButtonComponent>(entt::get<TransformComponent>);
			for(auto entity : group1){

				auto [transform, ui] = group1.get<TransformComponent, ButtonComponent>(entity);
				Renderer2D::DrawUI(transform.GetTransform(), ui, (int)entity);
			}

			ViewEntity<Entity, TextUIComponent>([this] (auto entity, auto& comp){

				auto& transform = entity.template GetComponent<TransformComponent>();	
				Renderer2D::DrawUI(transform.Translation, comp);		
			});		

			auto Spritegroup = m_Registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);
			for (auto entity : Spritegroup)
			{
				auto [transform, sprite] = Spritegroup.get<TransformComponent, SpriteRendererComponent>(entity);
				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
			}		

			// ViewEntity<Entity, SpriteRendererComponent>([this] (auto entity, auto& comp){

			// 	auto& transform = entity.template GetComponent<TransformComponent>();
			// 	Renderer2D::DrawSprite(transform.GetTransform(), comp, (int)entity);
			// });					

			Renderer2D::EndCamera();
		}

		ReadPixelEntity(mouseX, mouseY, viewportSize);

		m_Framebuffer->Unbind();

		RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        RenderCommand::Clear();        
		
    }

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera, int& mouseX, int& mouseY, glm::vec2& viewportSize)
	{
		m_Framebuffer->Bind();
		// Clear our entity ID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);
		RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    	RenderCommand::Clear();

		// m_Physics3D.Simulate(ts);
		// glEnable(GL_DEPTH_TEST);
		Renderer3D::BeginCamera(camera);
		//temp
		Renderer3D::RenderLight({5.5f, 5.0f, 0.3f });


		ViewEntity<Entity, ModelComponent>([this] (auto entity, auto& comp){
			auto& transform = entity.template GetComponent<TransformComponent>();
			Renderer3D::DrawModel(comp.ModelData, transform.GetTransform(), glm::vec3(1),(int)entity);
		});

		ViewEntity<Entity, CubeComponent>([this] (auto entity, auto& comp){			
			auto& transform = entity.template GetComponent<TransformComponent>();
			
			Renderer3D::DrawCube(transform.GetTransform(), comp.Color, (int)entity);
		});

		ViewEntity<Entity, CameraComponent>([this] (auto entity, auto& comp){
			auto& transform = entity.template GetComponent<TransformComponent>();
			comp.Camera.m_Position = &transform.Translation;
			// CamComp.Camera.m_Rotation2 = &transform.Rotation;
			if(ShowCams)
				Renderer3D::DrawCube(transform.GetTransform(), {1,0,0}, 1.0f, (int)entity - 1);
				// Renderer3D::DrawCameraFrustum(comp.Camera);
		});

		ViewEntity<Entity, BoxShapeComponent>([this] (auto entity, auto& comp){
			glm::vec3 minBounds(FLT_MAX);
			glm::vec3 maxBounds(-FLT_MAX);
			glm::vec3 center = glm::vec3(0);
			glm::vec3 halfExtents = glm::vec3(0);
			glm::vec3 scaledHalfExtents = glm::vec3(0);
			auto& transform = entity.template GetComponent<TransformComponent>();

			if(entity.HasComponent<ModelComponent>()){
				auto& modelComp = entity.GetComponent<ModelComponent>();

				for(auto& mesh : modelComp.ModelData->GetMeshes()){
					for(const auto& vertex : mesh->m_Vertices){
						minBounds = glm::min(minBounds, vertex.Position);
        				maxBounds = glm::max(maxBounds, vertex.Position);
					}
				}
			}

			if(entity.HasComponent<CubeComponent>()){
				auto& cubeComp = entity.GetComponent<CubeComponent>();

				for(const auto& vertex : Renderer3D::GetCubeMesh()->m_Vertices){
					minBounds = glm::min(minBounds, vertex.Position);
					maxBounds = glm::max(maxBounds, vertex.Position);
				}
			}

			center = (minBounds + maxBounds) * 0.5f;
			halfExtents = (maxBounds - minBounds) * 0.5f;

			// Apply scale if needed
			scaledHalfExtents = halfExtents * transform.Scale;

			if(!comp.Settings){
				comp.Settings =  new JPH::BoxShapeSettings{
					{ scaledHalfExtents.x, scaledHalfExtents.y, scaledHalfExtents.z }
				};
			}	

			JPH::Vec3 joltHalfExtents = { scaledHalfExtents.x, scaledHalfExtents.y, scaledHalfExtents.z };
			glm::vec3 joltCenterOffset = { center.x * transform.Scale.x, center.y * transform.Scale.y, center.z * transform.Scale.z };
			// Create offset transform
			JPH::Vec3 offset = {joltCenterOffset.x, joltCenterOffset.y, joltCenterOffset.z};
			JPH::Quat rotation = JPH::Quat::sIdentity(); // or use a glm-to-JPH conversion if needed
			
			auto boxSettings = new JPH::BoxShapeSettings(joltHalfExtents);

			auto offsetSettings = new JPH::RotatedTranslatedShapeSettings(offset, rotation, comp.Settings);

			JPH::ShapeSettings::ShapeResult shape_result = offsetSettings->Create();

			comp.Shape =  new JPH::BoxShape(*comp.Settings, shape_result);  

			if (shape_result.HasError())
			{
				UE_CORE_WARN("Error creating shape: {}",
							shape_result.GetError());
			}
			
			auto& box = comp.Shape->GetLocalBounds();

			if(ShowBoxes)
				Renderer3D::DrawWireCube({transform.Translation.x, transform.Translation.y, transform.Translation.z}, 
				{box.GetSize().GetX(), box.GetSize().GetY(), box.GetSize().GetZ()}, 
				{0,1,0}, 0.5f);	
				
			Renderer3D::SetEntity((int)entity);

		});		

		ViewEntity<Entity, SphereShapeComponent>([this] (auto entity, auto& comp){
			auto& transform = entity.template GetComponent<TransformComponent>();
			
			if(!comp.Settings){
				comp.Settings = new JPH::SphereShapeSettings{
					transform.GetRadius()
				};
			}

			JPH::ShapeSettings::ShapeResult shape_result = comp.Settings->Create();

			comp.Shape =  new JPH::SphereShape(*comp.Settings, shape_result);  

			if (shape_result.HasError())
			{
				UE_CORE_WARN("Error creating shape: {}",
							shape_result.GetError());
			}
			
			auto& box = comp.Shape->GetLocalBounds();

			if(ShowBoxes)
				Renderer3D::DrawWireSphere({transform.Translation.x, transform.Translation.y, transform.Translation.z}, 
				{box.GetSize().GetX(), box.GetSize().GetY(), box.GetSize().GetZ()}, 
				{0,0,1}, 0.5f);	
		});		

		Renderer3D::EndCamera();
        // glDisable(GL_DEPTH_TEST);
		Renderer2D::BeginCamera(camera);
		ViewEntity<Entity, UIElement>([this] (auto entity, auto& comp){

			auto& transform = entity.template GetComponent<TransformComponent>();			
			Renderer2D::DrawUI(transform.GetTransform(), comp);
		});

		auto group1 = m_Registry.group<ButtonComponent>(entt::get<TransformComponent>);
		for(auto entity : group1){

			auto [transform, ui] = group1.get<TransformComponent, ButtonComponent>(entity);
			Renderer2D::DrawUI(transform.GetTransform(), ui, (int)entity);
		}

		ViewEntity<Entity, TextUIComponent>([this] (auto entity, auto& comp){

			auto& transform = entity.template GetComponent<TransformComponent>();	
			Renderer2D::DrawUI(transform.Translation, comp);		
		});		

		auto Spritegroup = m_Registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);
		for (auto entity : Spritegroup)
		{
			auto [transform, sprite] = Spritegroup.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
		}		

		// ViewEntity<Entity, SpriteRendererComponent>([this] (auto entity, auto& comp){

		// 	auto& transform = entity.template GetComponent<TransformComponent>();
		// 	Renderer2D::DrawSprite(transform.GetTransform(), comp, (int)entity);
		// });					

		Renderer2D::EndCamera();		


		ReadPixelEntity(mouseX, mouseY, viewportSize);

		m_Framebuffer->Unbind();

		RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        RenderCommand::Clear();
        // DrawScreen(m_Framebuffer, camera);
	}

	void Scene::DrawScreen(Ref<Framebuffer>& buffer, EditorCamera& camera){		
		Renderer2D::BeginCamera(camera);
		Renderer2D::DrawScreen(buffer);
		Renderer2D::EndCamera();
	}

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		m_Framebuffer->Resize(width, height);

		// Resize our non-FixedAspectRatio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}

	}

	void Scene::OnMouseInput(float mouseX, float mouseY, bool mousePressed, Timestep ts){		
		auto group1 = m_Registry.group<ButtonComponent>(entt::get<TransformComponent>);
		for(auto entity : group1){

			auto [transform, button] = group1.get<TransformComponent, ButtonComponent>(entity);

			float halfWidth  = transform.Scale.x * 0.5f;
			float halfHeight = transform.Scale.y * 0.5f;

			bool hovered = mouseX >= (transform.Translation.x - halfWidth) &&
			mouseX <= (transform.Translation.x + halfWidth) &&
			mouseY >= (transform.Translation.y - halfHeight) &&
			mouseY <= (transform.Translation.y + halfHeight);

			button.Hovered = hovered;			

			if (hovered && mousePressed && !button.ClickedLastFrame) {				
				if (button.OnClick)
					button.OnClick();				
				button.ClickedLastFrame = true;
			}
			else if (!mousePressed) {
				button.ClickedLastFrame = false;
			}

			//Animate scale			
			if(button.ClickedLastFrame)
				button.TargetScale = button.OriginalScale * 0.95f;
			else if (button.Hovered)
				button.TargetScale = button.OriginalScale * 1.05f;
			else
				button.TargetScale = button.OriginalScale;
	
			transform.Scale = glm::mix(transform.Scale, button.TargetScale, 0.2f);

			button.BaseColor = button.Color;
			button.CurrentColor = button.Color;

			// Animate Color
			glm::vec4 hoverColor = button.BaseColor * 1.2f;
			hoverColor.a = button.BaseColor.a; // preserve alpha
	
			button.CurrentColor = glm::mix(button.CurrentColor, button.Hovered ? hoverColor : button.BaseColor, 0.2f);
			button.Color = button.CurrentColor;
	}
			
		m_MouseX = mouseX;
		m_MouseY = mouseY;
	}

	Entity Scene::GetHoveredEntity(){						

		if(GlobHovered)
			return GlobHovered;
		else
			return Entity();
	}

	void Scene::DuplicateEntity(Entity entity)
	{
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);

		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);		
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
	void  Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void  Scene::OnComponentAdded<UIElement>(Entity entity, UIElement& component)
	{
	}

	template<>
	void  Scene::OnComponentAdded<ButtonComponent>(Entity entity, ButtonComponent& component)
	{

	}

	template<>
	void  Scene::OnComponentAdded<TextUIComponent>(Entity entity, TextUIComponent& component)
	{
	}

	template<>
	void  Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void  Scene::OnComponentAdded<ModelComponent>(Entity entity, ModelComponent& component)
	{
	}

	template<>
	void  Scene::OnComponentAdded<CubeComponent>(Entity entity, CubeComponent& component)
	{
	}

	template<>
	void  Scene::OnComponentAdded<RigidbodyComponent>(Entity entity, RigidbodyComponent& component)
	{
	}

	template<>
	void  Scene::OnComponentAdded<BoxShapeComponent>(Entity entity, BoxShapeComponent& component)
	{
	}

	template<>
	void  Scene::OnComponentAdded<SphereShapeComponent>(Entity entity, SphereShapeComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CharacterComponent>(Entity entity, CharacterComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}
}

#include "uepch.h"
#include "Scene/Scene.h"
#include "Scene/Components.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Renderer3D.h"
#include "Renderer/RenderCommand.h"
#include <glm/glm.hpp>
#include "UE_Assert.h"
#include "Scene/Entity.h"
//temp
#include <glad/glad.h>


namespace UE {

	Entity GlobHovered;

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
    Scene::~Scene(){}

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
		// CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);		

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

    void Scene::OnUpdateRuntime(Timestep ts, int& mouseX, int& mouseY, glm::vec2& viewportSize){
		// m_Framebuffer->Bind();
		// // Clear our entity ID attachment to -1
		// m_Framebuffer->ClearAttachment(1, -1);
		// RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    	// RenderCommand::Clear();

		// m_Physics3D.Simulate(ts);
		// // glEnable(GL_DEPTH_TEST);
		// Renderer3D::BeginCamera(m_Cam3D);
		// //temp
		// Renderer3D::RenderLight({5.5f, 5.0f, 0.3f });

		// auto modelGroup = m_Registry.group<ModelComponent>(entt::get<TransformComponent>);
		// for (auto entity : modelGroup) {
		// 	auto [transform, modelComp] = modelGroup.get<TransformComponent, ModelComponent>(entity);
			
		// 	if(modelComp.AnimationData){
		// 		// modelComp.AnimationData->SetModel(modelComp.ModelData);
		// 		Renderer3D::RunAnimation(modelComp.AnimationData, ts);
		// 	}
			
		// 	Renderer3D::DrawModel(modelComp.ModelData, transform.GetTransform(), glm::vec3(1),(int)entity);			
		// }

		// auto rigidBodyGroup = m_Registry.group<RigidbodyComponent>(entt::get<TransformComponent>);
		// for(auto entity : rigidBodyGroup){
		// 	auto& [transform, rigidBodyComp] = rigidBodyGroup.get<TransformComponent, RigidbodyComponent>(entity);			

		// 	const physx::PxTransform& pxTransform1 = rigidBodyComp.Body->getGlobalPose();
		// 	transform.Translation = { pxTransform1.p.x, pxTransform1.p.y, pxTransform1.p.z };
		// 	transform.Rotation = { pxTransform1.q.x, pxTransform1.q.y, pxTransform1.q.z};
		// 	// , pxTransform.q.w }
		// }

		// auto CubeGroup = m_Registry.group<CubeComponent>(entt::get<TransformComponent>);
		// for (auto entity : CubeGroup) {
		// 	auto [transform, CubeComp] = CubeGroup.get<TransformComponent, CubeComponent>(entity);
			
		// 	Renderer3D::DrawCube(transform.GetTransform(), CubeComp.Color, (int)entity);
		// }

		// Renderer3D::EndCamera();
        // // glDisable(GL_DEPTH_TEST);
		// Renderer2D::BeginCamera(m_Cam);
		// ViewEntity<Entity, UIElement>([this] (auto entity, auto& comp){

		// 	auto& transform = entity.template GetComponent<TransformComponent>();			
		// 	Renderer2D::DrawUI(transform.GetTransform(), comp);
		// });

		// auto group1 = m_Registry.group<ButtonComponent>(entt::get<TransformComponent>);
		// for(auto entity : group1){

		// 	auto [transform, ui] = group1.get<TransformComponent, ButtonComponent>(entity);
		// 	Renderer2D::DrawUI(transform.GetTransform(), ui, (int)entity);
		// }

		// ViewEntity<Entity, TextUIComponent>([this] (auto entity, auto& comp){

		// 	auto& transform = entity.template GetComponent<TransformComponent>();	
		// 	Renderer2D::DrawUI(transform.Translation, comp);		
		// });		

		// auto Spritegroup = m_Registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);
		// for (auto entity : Spritegroup)
		// {
		// 	auto [transform, sprite] = Spritegroup.get<TransformComponent, SpriteRendererComponent>(entity);
		// 	Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
		// }		

		// // ViewEntity<Entity, SpriteRendererComponent>([this] (auto entity, auto& comp){

		// // 	auto& transform = entity.template GetComponent<TransformComponent>();
		// // 	Renderer2D::DrawSprite(transform.GetTransform(), comp, (int)entity);
		// // });					

		// Renderer2D::EndCamera();		

		// auto my = m_MouseY;
		// auto mx = m_MouseX;

		// my = m_ViewportHeight - my;			

		// if(mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y){            
		// 	int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
		// 	// UE_INFO("mx: {0}, my: {1}" ,Input::GetMouseX(), Input::GetMouseY() );
		// 	if(pixelData < -1 || pixelData >= 1036831949)
		// 		pixelData = -1;
		// 	// UE_INFO("Pixel {0}",pixelData);
		// 	GlobHovered = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, this);            
		// }

		// m_Framebuffer->Unbind();

		// RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        // RenderCommand::Clear();
        // DrawScreen(m_Framebuffer);
		
    }

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera, int& mouseX, int& mouseY, glm::vec2& viewportSize)
	{
		m_Framebuffer->Bind();
		// Clear our entity ID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);
		RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    	RenderCommand::Clear();

		m_Physics3D.Simulate(ts);
		// glEnable(GL_DEPTH_TEST);
		Renderer3D::BeginCamera(camera);
		//temp
		Renderer3D::RenderLight({5.5f, 5.0f, 0.3f });

		auto modelGroup = m_Registry.group<ModelComponent>(entt::get<TransformComponent>);
		for (auto entity : modelGroup) {
			auto [transform, modelComp] = modelGroup.get<TransformComponent, ModelComponent>(entity);
			
			if(modelComp.AnimationData){
				// modelComp.AnimationData->SetModel(modelComp.ModelData);
				Renderer3D::RunAnimation(modelComp.AnimationData, ts);
			}
			
			Renderer3D::DrawModel(modelComp.ModelData, transform.GetTransform(), glm::vec3(1),(int)entity);			
		}

		auto rigidBodyGroup = m_Registry.group<RigidbodyComponent>(entt::get<TransformComponent>);
		for(auto entity : rigidBodyGroup){
			auto& [transform, rigidBodyComp] = rigidBodyGroup.get<TransformComponent, RigidbodyComponent>(entity);			

			const physx::PxTransform& pxTransform1 = rigidBodyComp.Body->getGlobalPose();
			transform.Translation = { pxTransform1.p.x, pxTransform1.p.y, pxTransform1.p.z };
			transform.Rotation = { pxTransform1.q.x, pxTransform1.q.y, pxTransform1.q.z};
			// , pxTransform.q.w }
		}

		auto CubeGroup = m_Registry.group<CubeComponent>(entt::get<TransformComponent>);
		for (auto entity : CubeGroup) {
			auto [transform, CubeComp] = CubeGroup.get<TransformComponent, CubeComponent>(entity);
			
			Renderer3D::DrawCube(transform.GetTransform(), CubeComp.Color, (int)entity);
		}

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

		auto my = m_MouseY;
		auto mx = m_MouseX;

		my = m_ViewportHeight - my;			

		if(mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y){            
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			// UE_INFO("mx: {0}, my: {1}" ,Input::GetMouseX(), Input::GetMouseY() );
			if(pixelData < -1 || pixelData >= 1036831949)
				pixelData = -1;
			// UE_INFO("Pixel {0}",pixelData);
			GlobHovered = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, this);            
		}

		m_Framebuffer->Unbind();

		RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        RenderCommand::Clear();
        DrawScreen(m_Framebuffer, camera);
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
}

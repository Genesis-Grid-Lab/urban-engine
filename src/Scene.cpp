#include "Scene/Scene.h"

#include "Scene/Components.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RenderCommand.h"
#include <glm/glm.hpp>
#include "UE_Assert.h"
#include "Scene/Entity.h"


namespace UE {

	Entity GlobHovered;

    Scene::Scene(uint32_t width, uint32_t height){
		m_Cam = Camera(0.0f, width, height, 0.0f);

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = width;
        fbSpec.Height = height;
        m_Framebuffer = CreateRef<Framebuffer>(fbSpec);
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

    void Scene::OnUpdateRuntime(Timestep ts){
		m_Framebuffer->Bind();
		// Clear our entity ID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);
		RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    	RenderCommand::Clear();
        
		Renderer2D::BeginCamera(m_Cam);
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

		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
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

		if(m_MouseX >= 0 && m_MouseY >= 0 && m_MouseX < m_ViewportWidth && m_MouseY < m_ViewportHeight){            
			int pixelData = m_Framebuffer->ReadPixel(1, mx, my);
			// UE_INFO("Pixel {0}",pixelData);
			// UE_INFO("mx: {0}, my: {1}" ,Input::GetMouseX(), Input::GetMouseY() );
			GlobHovered = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, this);            
		}

		m_Framebuffer->Unbind();

		RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        RenderCommand::Clear();
        DrawScreen(m_Framebuffer);
		
    }

	void Scene::DrawScreen(Ref<Framebuffer>& buffer){
		Renderer2D::BeginCamera(m_Cam);
		Renderer2D::DrawScreen(buffer);
		// Renderer2D::EndCamera();
	}

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// m_Cam.SetProjection(0.0f, width, height, 0.0f);

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
	void UE_API Scene::OnComponentAdded(Entity entity, T& component)
	{
		// static_assert(false);
	}

	template<>
	void UE_API Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	}

	template<>
	void UE_API Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void UE_API Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void UE_API Scene::OnComponentAdded<UIElement>(Entity entity, UIElement& component)
	{
	}

	template<>
	void UE_API Scene::OnComponentAdded<ButtonComponent>(Entity entity, ButtonComponent& component)
	{

	}

	template<>
	void UE_API Scene::OnComponentAdded<TextUIComponent>(Entity entity, TextUIComponent& component)
	{
	}

	template<>
	void UE_API Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}
}

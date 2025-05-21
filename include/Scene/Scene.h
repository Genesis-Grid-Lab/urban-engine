#pragma once

#include "Config.h"
#include "Core/Timestep.h"
#include "Core/UUID.h"
#include <entt.hpp>
#include "Renderer/Framebuffer.h"
namespace UE {

    class Entity;

    class  Scene{
    public:
        Scene();
        Scene(uint32_t width, uint32_t height);
        ~Scene();

        static Ref<Scene> Copy(Ref<Scene> other);

        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
        void DestroyEntity(Entity entity);      
        template<typename Entt, typename Comp, typename Task>
        void ViewEntity(Task&& task){
            // UE_CORE_ASSERT(std::is_base_of<Entity, Entt>::value, "error viewing entt");
            m_Registry.view<Comp>().each([this, &task] 
            (const auto entity, auto& comp) 
            { 
                // task(std::move(Entt(&m_Registry, entity)), comp);
                task(std::move(Entt(entity, this)), comp);
            });
        }

        void OnRuntimeStart();
		void OnRuntimeStop();
        // void PhysicsUpdate(float dt);

        void OnUpdateRuntime(Timestep ts);  
        void OnUpdateEditor(Timestep ts);

        void DuplicateEntity(Entity entity);

        entt::registry& GetRegistry() { return m_Registry;}
       
        bool ShowBoxes = false;
        bool ShowCams = true;
        bool ShowBoxesPlay = false;
        Scope<Framebuffer> m_Buffer;
    private:
        template<typename T>
        void  OnComponentAdded(Entity entity, T& component);
    private:
        // void ReadPixelEntity(int& mouseX, int& mouseY, glm::vec2& viewportSize);
    private:
        entt::registry m_Registry;
        Camera3D camera = {0};
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
        float m_MouseX, m_MouseY;
        friend class Entity;                
        // friend class SceneSerializer;    
    };
}

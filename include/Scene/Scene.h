#pragma once

#include "Config.h"
#include "Timestep.h"
#include "UUID.h"
#include "Renderer/Camera.h"
#include "Renderer/Texture.h"
#include "UE_Assert.h"
#include <entt.hpp>
#include "Renderer/Shader.h"
#include "Renderer/Model.h"

namespace UE {

    class Entity;

    class UE_API Scene{
    public:
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

        void OnUpdateRuntime(Timestep ts);  
        void DrawScreen(Ref<Framebuffer>& buffer);          
        void OnViewportResize(uint32_t width, uint32_t height);   
        void OnMouseInput(float mouseX, float mouseY, bool mousePressed, Timestep ts);     

        void DuplicateEntity(Entity entity);

        Entity GetHoveredEntity();

        Camera& GetCam() { return m_Cam;}
        // Ref<Framebuffre>& GetBuffer() { return m_Framebuffer;}
        //temp
        Ref<Framebuffer> m_Framebuffer;
    private:
        template<typename T>
        void UE_API OnComponentAdded(Entity entity, T& component);
    private:
        entt::registry m_Registry;
        Ref<Texture2D> m_Screen;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
        Camera2D m_Cam;
        float m_MouseX, m_MouseY;
        friend class Entity;                
        // friend class SceneSerializer;    
    //temp
    private:
        Ref<Shader> TesShader;
        Ref<Model> TesModel;
    };
}

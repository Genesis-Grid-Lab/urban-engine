#pragma once

#include "Entity.h"
#include "Auxiliaries/Physics.h"

namespace UE {

	class ScriptableEntity
	{
	public:
		virtual ~ScriptableEntity() {}

		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

        Scene* GetScene(){
            return m_Scene;
        }

        JPH::BodyInterface &GetBodyInterface() { return *m_BodyInterface;}
	protected:
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
	private:
		Entity m_Entity;
        Scene* m_Scene;
        JPH::BodyInterface *m_BodyInterface;
		friend class Scene;
		friend class SceneHierarchyPanel;
	};

}


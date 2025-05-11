#pragma once

#include "Core/Config.h"
#include <PxPhysicsAPI.h>

namespace UE {
    class PhysicsSystem {
    public:
        void Init();
        void Shutdown();
        void Simulate(float timestep);
    
        physx::PxPhysics* GetPhysics() const { return m_Physics; }
        physx::PxScene* GetScene() const { return m_Scene; }
    
    private:
        physx::PxFoundation* m_Foundation = nullptr;
        physx::PxPhysics* m_Physics = nullptr;
        physx::PxScene* m_Scene = nullptr;
        // physx::PxDispatcher* m_Dispatcher = nullptr;
        physx::PxDefaultCpuDispatcher* m_CpuDispatcher = nullptr;
        physx::PxDefaultAllocator m_Allocator;
        physx::PxDefaultErrorCallback m_ErrorCallback;
    };
}
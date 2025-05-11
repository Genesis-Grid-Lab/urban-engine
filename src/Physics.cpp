#include "Auxiliaries/Physics.h"

namespace UE {

    void PhysicsSystem::Init(){

        m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
        m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, physx::PxTolerancesScale(), true);

        physx::PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
        sceneDesc.gravity = { 0.0f, -9.81f, 0.0f};        
        m_CpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
        sceneDesc.cpuDispatcher = m_CpuDispatcher;
        sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
        
        m_Scene = m_Physics->createScene(sceneDesc);
    }

    void PhysicsSystem::Shutdown() {
        m_Scene->release();
        m_CpuDispatcher->release();
        m_Physics->release();
        m_Foundation->release();
    }

    void PhysicsSystem::Simulate(float timestep) {
        m_Scene->simulate(timestep);
        m_Scene->fetchResults(true);
    }
}
#pragma once
#include "Config.h"
#include "Timestep.h"
#include "UUID.h"
#include "Components.h"
#include "Auxiliaries/Physics.h"
#include "Scene.h"

namespace UE {

    class RuntimeScene : public Scene {
    public:
        RuntimeScene();
        virtual ~RuntimeScene() override;

        void OnRuntimeStart();
        void OnRuntimeStop();
        void PhysicsUpdate(float dt);
        virtual void OnUpdate(Timestep ts) override;

        PhysicsEngine m_Physics3D;
    };
}
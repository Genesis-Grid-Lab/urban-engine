// #include "Auxiliaries/Physics.h"
// #include <Jolt/Jolt.h>
// #include <Jolt/Core/Core.h>
// #include <Jolt/Core/Factory.h>
// #include <Jolt/Core/IssueReporting.h>
// #include <Jolt/Core/JobSystemThreadPool.h>
// #include <Jolt/Core/TempAllocator.h>
// #include <Jolt/Math/Quat.h>
// #include <Jolt/Math/Real.h>
// #include <Jolt/Math/Vec3.h>
// #include <Jolt/Physics/Body/Body.h>
// #include <Jolt/Physics/Body/BodyActivationListener.h>
// #include <Jolt/Physics/Body/BodyCreationSettings.h>
// #include <Jolt/Physics/Body/BodyInterface.h>
// #include <Jolt/Physics/Body/MotionType.h>
// #include <Jolt/Physics/Collision/ContactListener.h>
// #include <Jolt/Physics/Collision/Shape/BoxShape.h>
// #include <Jolt/Physics/Collision/Shape/SphereShape.h>
// #include <Jolt/Physics/EActivation.h>
// #include <Jolt/Physics/PhysicsSettings.h>
// #include <Jolt/Physics/PhysicsSystem.h>
// #include <Jolt/RegisterTypes.h>

// using namespace JPH::literals;

// namespace UE {

//   // Callback for traces, connect this to your own trace function if you have
//   one static void TraceImpl(const char *inFMT, ...)
//   {
//     // Format the message
//     va_list list;
//     va_start(list, inFMT);
//     char buffer[1'024];
//     vsnprintf(buffer, sizeof(buffer), inFMT, list);
//     va_end(list);

//     // Print to the TTY
//     std::cout << "[Jolt]" << buffer << std::endl;
//   }

// #ifdef JPH_ENABLE_ASSERTS

//   // Callback for asserts, connect this to your own assert handler if you
//   have one static bool AssertFailedImpl(const char *inExpression,
// 			       const char *inMessage,
// 			       const char *inFile,
// 			       unsigned int inLine)
//   {
//     // Print to the TTY
//     std::cout << inFile << ":" << inLine << ": (" << inExpression << ") "
// 	      << (inMessage != nullptr ? inMessage : "") << std::endl;

//     // Breakpoint
//     return true;
//   };

// #endif // JPH_ENABLE_ASSERTS

//   // ---------- static bootstrap ----------
//   std::atomic<int> PhysicsEngine::s_joltInits{0};

//   void PhysicsEngine::InitJoltOnce() {
//     if (s_joltInits.fetch_add(1, std::memory_order_acq_rel) == 0) {
//       JPH::RegisterDefaultAllocator();
//       JPH::Trace = TraceImpl;
//       JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)
//         JPH::Factory::sInstance = new JPH::Factory();
//       JPH::RegisterTypes();
//     }
//   }

//   void PhysicsEngine::ShutdownJoltOnce() {
//     if (s_joltInits.fetch_sub(1, std::memory_order_acq_rel) == 1) {
//       JPH::UnregisterTypes();
//       delete JPH::Factory::sInstance;
//       JPH::Factory::sInstance = nullptr;
//     }
//   }

//   // ---------- engine ----------
//   PhysicsEngine::PhysicsEngine() = default;
//   PhysicsEngine::~PhysicsEngine() { Shutdown();}

//     void PhysicsEngine::Init(){

//       if (_physics)
//         return;

//       InitJoltOnce();

//       // allocators / jobs
//     _temp_alloc = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
//     const int hw = (int)std::thread::hardware_concurrency();
//     const int num_threads   = std::max(1, hw - 1);
//     const JPH::uint maxJobs = 1024;
//     const JPH::uint maxBars = 8;

//     _jobs = std::make_unique<JPH::JobSystemThreadPool>(maxJobs, maxBars,
//     num_threads);

//     // create physics system
//     _physics = std::make_unique<JPH::PhysicsSystem>();

//     // NOTE: tune these for your game
//     constexpr uint cMaxBodies               = 1024;
//     constexpr uint cNumBodyMutexes          = 0;
//     constexpr uint cMaxBodyPairs            = 1024;
//     constexpr uint cMaxContactConstraints   = 1024;

//     // _debugRenderer = std::make_unique<UEJoltDebugRenderer>();
//     // JPH::DebugRenderer::sInstance = _debugRenderer.get();

//     _physics->Init(
//         cMaxBodies,
//         cNumBodyMutexes,
//         cMaxBodyPairs,
//         cMaxContactConstraints,
//         *_bp_iface,
//         *_obj_vs_bp_filter,
//         *_obj_vs_obj_filter
//     );

//     // example: floor (optional)
//     // {
//     //     BodyInterface& bi = Bodies();
//     //     RefConst<Shape> floor_shape = new
//     BoxShape(Vec3(50.0f, 1.0f, 50.0f));
//     //     BodyCreationSettings bcs(floor_shape, RVec3(0,-1,0),
//     Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
//     //     BodyID floor = bi.CreateAndAddBody(bcs,
//     EActivation::DontActivate);
//     //     (void)floor;
//     // }
//     }

//   bool PhysicsEngine::Step(float dt){
//     if (!_physics) return false;
//     _accumulator += dt;
//     bool simulated = false;

//     // clamp to avoid spiral of death
//     const float maxFrame = 0.25f;
//     if (_accumulator > maxFrame) _accumulator = maxFrame;

//     while (_accumulator >= _fixedStep) {
//       _physics->Update(_fixedStep, 1, _temp_alloc.get(), _jobs.get());
//       _accumulator -= _fixedStep;
//       simulated = true;
//     }
//     UE_CORE_INFO("[PHYSICS ENGINE]: {}", _accumulator);
//     return simulated;
//   }

//   void PhysicsEngine::StartSimulation() {
//     if(_physics) _physics->OptimizeBroadPhase();
//   }

//   JPH::BodyInterface& PhysicsEngine::Bodies() {
//     // if you run multi-threaded simulation, prefer GetBodyInterface();
//     otherwise NoLock is fine return _physics->GetBodyInterface();
//     // return _physics->GetBodyInterfaceNoLock();
//   }

//     void PhysicsEngine::Shutdown(){

//         // Unregisters all types with the factory and cleans up the default
//         material JPH::UnregisterTypes();

//         // Destroy the factory
//         delete JPH::Factory::sInstance;
//         JPH::Factory::sInstance = nullptr;
//     }

//   JPH::JobSystem& PhysicsEngine::JobSystem()       { return *_jobs; } //
//   ThreadPool is-a JobSystem JPH::TempAllocator&
//   PhysicsEngine::TempAllocator() { return *_temp_alloc; }  // Impl is-a
//   TempAllocator
// }

#include "Auxiliaries/Physics.h"
#include "Components.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/Memory.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/RegisterTypes.h"
#include "Log.h"
#include "Runtime/RuntimeScene.h"

namespace UE {

std::unique_ptr<BPLayerInterfaceImpl> Physics::_bp_iface;
std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> Physics::_obj_vs_bp_filter;
std::unique_ptr<ObjectLayerPairFilterImpl> Physics::_obj_vs_obj_filter;

std::unique_ptr<JPH::TempAllocatorImpl> Physics::_temp_alloc;
std::unique_ptr<JPH::JobSystemThreadPool> Physics::_jobs;

std::unique_ptr<MyContactListener> Physics::_contact_listener;
std::unique_ptr<MyBodyActivationListener> Physics::_activation_listener;

void Physics::Init() {
  JPH::RegisterDefaultAllocator();
  JPH::Factory::sInstance = new JPH::Factory();
  JPH::RegisterTypes();

  // interfaces + filters (must outlive PhysicsSystem)
  _bp_iface = std::make_unique<BPLayerInterfaceImpl>();
  _obj_vs_bp_filter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
  _obj_vs_obj_filter = std::make_unique<ObjectLayerPairFilterImpl>();
  _contact_listener = std::make_unique<MyContactListener>();
  _activation_listener = std::make_unique<MyBodyActivationListener>();

  const uint cMaxBodies = 65536;
  const uint cNumBodyMutexes = 8;
  const uint cMaxBodyPairs = 65536;
  const uint cMaxContactConstraints = 10240;

  _temp_alloc = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
  const int hw = (int)std::thread::hardware_concurrency();
  const int num_threads = std::max(1, hw - 1);
  const JPH::uint maxJobs = 1024;
  const JPH::uint maxBars = 8;

  _jobs =
      std::make_unique<JPH::JobSystemThreadPool>(maxJobs, maxBars, num_threads);

  // gravity etc.
  const JPH::Vec3 gravity(0.0f, -9.81f, 0.0f);

  m_PhysicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs,
                       cMaxContactConstraints, *_bp_iface, *_obj_vs_bp_filter,
                       *_obj_vs_obj_filter);

  m_PhysicsSystem.SetGravity(gravity);
  m_PhysicsSystem.SetBodyActivationListener(_activation_listener.get());
  m_PhysicsSystem.SetContactListener(_contact_listener.get());
}

void Physics::CreateBody(entt::entity entity, RuntimeScene *scene) {
  auto &rb = scene->GetRegistry().get<RigidbodyComponent>(entity);
  auto &transform = scene->GetRegistry().get<TransformComponent>(entity);
  auto &collider = scene->GetRegistry().get<BoxColliderComponent>(entity);
  auto &name = scene->GetRegistry().get<TagComponent>(entity);

  UE_CORE_WARN("[PHYSICS] Creating body for {}", name.Tag);

  JPH::BodyInterface &bodyInterface = m_PhysicsSystem.GetBodyInterface();

  JPH::BoxShapeSettings shapeSettings(
      JPH::Vec3(collider.HalfSize.x, collider.HalfSize.y, collider.HalfSize.z));

  JPH::ShapeRefC shape = shapeSettings.Create().Get();

  UE_CORE_WARN("Entity start position {} {} {}", transform.Translation.x,
               transform.Translation.y, transform.Translation.z);

  JPH::BodyCreationSettings bodySettings(
      shape,
      JPH::RVec3(transform.Translation.x, transform.Translation.y,
                 transform.Translation.z),
      JPH::Quat::sIdentity(),
      rb.Type == RigidbodyComponent::BodyType::Dynamic
          ? JPH::EMotionType::Dynamic
          : JPH::EMotionType::Static,
      0);

  bodySettings.mGravityFactor = rb.UseGravity ? 1.0f : 0.0f;

  JPH::BodyID id =
      bodyInterface.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);

  UE_CORE_WARN("[PHYSICS] body active: {}", bodyInterface.IsActive(id));

  rb.BodyID = id.GetIndexAndSequenceNumber();

  rb.RuntimeCreated = true;

  UE_CORE_WARN("[PHYSICS] body created for {} with type {}", rb.BodyID,
               (int)rb.Type);
}

void Physics::Step(RuntimeScene *scene, float dt) {
  UE_CORE_WARN("[PHYSICS] step dt = {}", dt);
  m_PhysicsSystem.Update(dt, 1, _temp_alloc.get(), _jobs.get());

  auto view =
      scene->GetRegistry().view<RigidbodyComponent, TransformComponent>();

  for (auto entity : view) {
    auto &rb = view.get<RigidbodyComponent>(entity);
    auto &transform = view.get<TransformComponent>(entity);

    if (!rb.RuntimeCreated || rb.Type == RigidbodyComponent::BodyType::Static)
      continue;

    JPH::BodyInterface &bodyInterface = m_PhysicsSystem.GetBodyInterface();

    JPH::BodyID id(rb.BodyID);

    JPH::RVec3 pos = bodyInterface.GetPosition(id);

    UE_CORE_WARN("Body position {} {} {}", pos.GetX(), pos.GetY(), pos.GetZ());

    transform.Translation = {pos.GetX(), pos.GetY(), pos.GetZ()};
  }
}

void Physics::Shutdown() {
  // m_PhysicsSystem = JPH::PhysicsSystem();

  _activation_listener.reset();
  _contact_listener.reset();

  _bp_iface.reset();
  _obj_vs_bp_filter.reset();
  _obj_vs_obj_filter.reset();

  _jobs.reset();
  _temp_alloc.reset();
}
} // namespace UE
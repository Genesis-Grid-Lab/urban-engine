#include "Auxiliaries/Physics.h"
#include "entity/fwd.hpp"
#include "uepch.h"
#include "Scene/Scene.h"
#include "Scene/SceneCamera.h"
#include "Scene/Components.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Renderer3D.h"
#include "Renderer/RenderCommand.h"
#include <cstdint>
#include <glm/glm.hpp>
#include "UE_Assert.h"
#include "Scene/Entity.h"
#include "Scene/ScriptableEntity.h"
#include "Core/Log.h"
#include <Jolt/Math/Math.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/CompoundShape.h>
#include <Jolt/Physics/Collision/Shape/SubShapeID.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CollisionCollector.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

//temp
#include <glad/glad.h>


namespace UE {

  // Local helpers
  static inline JPH::RefConst<JPH::Shape> BuildBox(const glm::vec3& he) {
    auto res = JPH::BoxShapeSettings(JPH::Vec3(he.x, he.y, he.z)).Create();
    return res.IsValid() ? res.Get() : nullptr;
  }
  static inline JPH::RefConst<JPH::Shape> BuildSphere(float r) {
    auto res = JPH::SphereShapeSettings(r).Create();
    return res.IsValid() ? res.Get() : nullptr;
  }

  static inline JPH::RefConst<JPH::Shape> BuildCapsule(float half_h, float r) { 
    auto res = JPH::CapsuleShapeSettings(half_h, r).Create();
    return res.IsValid() ? res.Get() : nullptr;
  }
  //to change
  static inline void DestroyBodyIfAny(PhysicsEngine& phys, RigidbodyComponent& rb) {
    if (rb.ID.IsInvalid()) return;
    auto& bi = phys.Bodies();
    bi.RemoveBody(rb.ID);
    bi.DestroyBody(rb.ID);
    rb.ID = {};
  }

  static inline glm::quat ToGlmQuat(const JPH::Quat& q) {
    return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ()); // glm wants (w,x,y,z)
  }
  static inline JPH::Quat ToJoltQuat(const glm::quat& q) {
    return JPH::Quat(q.x, q.y, q.z, q.w); // Jolt ctor is (x,y,z,w)
  }

  static inline JPH::Vec3 ToJoltVec3(const glm::vec3& v){
    return JPH::Vec3(v.x, v.y, v.z);
  }

  static inline void FromJoltTransform(const JPH::RMat44& xf, glm::vec3& outPos, glm::vec3& outEuler)
  {
    const JPH::RVec3 t = xf.GetTranslation();
    outPos = glm::vec3((float)t.GetX(), (float)t.GetY(), (float)t.GetZ());

    const JPH::Quat rq = xf.GetRotation().GetQuaternion();  // Jolt rotation
    const glm::quat gq((float)rq.GetW(), (float)rq.GetX(), (float)rq.GetY(), (float)rq.GetZ());
    outEuler = glm::eulerAngles(gq); // radians
  }

  Entity GlobHovered;

  Scene::Scene() {

  }

  Scene::Scene(uint32_t width, uint32_t height){
    UE_PROFILE_FUNCTION();
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

  void Scene::DestroyEntityNow(Entity entity) { m_Registry.destroy(entity); }

  void Scene::DestroyEntity(Entity entity){
    // Queue for destruction; actual registry destroy happens in
    // FlushEntityDestruction()
    m_DestroyQueue.push_back((entt::entity)entity);
  }

  void Scene::ReadPixelEntity(int& mouseX, int& mouseY, glm::vec2& viewportSize){
    if(mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y){            
      int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
      // UE_INFO("mx: {0}, my: {1}" ,Input::GetMouseX(), Input::GetMouseY() );
      if(pixelData < -1 || pixelData >= 99036831949)
	pixelData = -1;
      // UE_INFO("Pixel {0}",pixelData);
      GlobHovered = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, this);            
    }
  }

  template<typename ShapeComp>
  void Scene::BuildRigidBodyForEntity(Entity e, TransformComponent& tr, ShapeComp& sc)
  {
    if (!sc.Shape) {
      UE_CORE_ERROR("[BuildRigidBodyForEntity]: No Shape in RigidBodyComponent: {}", e.GetName());
      return;    
    }

    auto& rb = e.GetComponent<RigidbodyComponent>();

    JPH::EMotionType motion = JPH::EMotionType::Dynamic;
    JPH::ObjectLayer layer  = Layers::MOVING;
    switch (rb.Type) {
    case BodyType::Static:    motion = JPH::EMotionType::Static;    layer = Layers::NON_MOVING; break;
    case BodyType::Kinematic: motion = JPH::EMotionType::Kinematic; layer = Layers::MOVING;     break;
    case BodyType::Dynamic:
      motion = JPH::EMotionType::Dynamic;
      layer = Layers::MOVING;
      break;
    default:
      UE_CORE_WARN("[BuildRigidBodyForEntity]: unknown BodyType");
      break;
    }

    UE_CORE_INFO("[BuildRigidBodyForEntity glm pos] {},{},{}", tr.Translation.x, tr.Translation.y, tr.Translation.z);
    const JPH::RVec3 pos(tr.Translation.x, tr.Translation.y, tr.Translation.z);
    UE_CORE_INFO("[BuildRigidBodyForEntity JPH pos] {},{},{}", pos.GetX(), pos.GetY(), pos.GetZ());
    const glm::quat gp = glm::quat(tr.Rotation);
    const JPH::Quat  rot = ToJoltQuat(gp);

    JPH::BodyCreationSettings bcs(sc.Shape, pos, rot, motion, layer);
    bcs.mAllowSleeping   = true;
    bcs.mLinearDamping   = rb.LinearDamp;
    bcs.mAngularDamping  = rb.AngularDamp;
    bcs.mGravityFactor   = rb.GravityFactor;
    bcs.mMotionQuality   = rb.Continuous ? JPH::EMotionQuality::LinearCast
      : JPH::EMotionQuality::Discrete;
    bcs.mUserData = (JPH::uint64)(entt::entity)e; // map contacts -> entity

    auto &bi = m_Physics3D.Bodies();
    auto &sys = m_Physics3D.System();
    rb.ID = bi.CreateAndAddBody(bcs, JPH::EActivation::Activate);
    if (rb.ID.IsInvalid()) {
      UE_CORE_ERROR("[BuildRigidBodyForEntity]: CreateAndAddBody failed");
      return;
    }

    if (rb.Type == BodyType::Dynamic && rb.Mass > 0.0f) {
      // Optional: set mass properties (or leave Jolt to compute from shape density)
      bi.SetLinearVelocity(rb.ID, JPH::Vec3(0,0,0));
      bi.SetAngularVelocity(rb.ID, JPH::Vec3(0,0,0));
    }

    // Extra sanity: verify the body truly exists & inspect it.
    {
      const JPH::BodyLockInterfaceNoLock &bli = sys.GetBodyLockInterfaceNoLock();
      JPH::BodyLockRead lock(bli, rb.ID);
      if (!lock.Succeeded()) {
        UE_CORE_ERROR("[BuildRigidBodyForEntity]: CreateAndAddBody - Body lock failed right after creation (not added?)");
        return;
      }
      const JPH::Body &b = lock.GetBody();
      UE_CORE_INFO("[BuildRigidBodyForEntity]: Body created - id={} motion={} grav={} added={} active={}",
		   (unsigned)rb.ID.GetIndexAndSequenceNumber(),
		   (int)b.GetMotionType(),
		   8,
		   bi.IsAdded(rb.ID) ? 1 : 0,
		   bi.IsActive(rb.ID) ? 1 : 0);
    }

    if (!bi.IsAdded(rb.ID)) {
	UE_CORE_ERROR("[BuildRigidBodyForEntity]: {} is not Added with id {}", e.GetName(), (unsigned)rb.ID.GetIndexAndSequenceNumber());
        return;
    }else {
      UE_CORE_INFO("[CREATRE] {} Added with id {}", e.GetName(), (unsigned)rb.ID.GetIndexAndSequenceNumber());
        }
  }

  template<typename TChar>
    void Scene::BuildCharacterForEntity(Entity e, TransformComponent& tr, TChar& cc) {
    if (!cc.Shape) return;

    JPH::BodyCreationSettings bcs(
				  cc.Shape,
				  JPH::RVec3(tr.Translation.x, tr.Translation.y, tr.Translation.z),
				  ToJoltQuat(glm::quat(tr.Rotation)),
				  JPH::EMotionType::Dynamic,
				  Layers::CHARACTER
				  );

    bcs.mFriction      = 0.8f;
    bcs.mRestitution   = 0.0f;
    bcs.mMotionQuality = JPH::EMotionQuality::LinearCast; // <-- force sweep
    bcs.mIsSensor      = false;                           // <-- must collide
    bcs.mUserData      = (JPH::uint64)(entt::entity)e;

    auto& bi = m_Physics3D.Bodies();
    cc.Body = bi.CreateAndAddBody(bcs, JPH::EActivation::Activate);
    cc.SpawnSynced = false;
  }


  // ------------------------------
  // Scene::OnRuntimeStart
  // ------------------------------
  void Scene::OnRuntimeStart()
  {
    UE_PROFILE_FUNCTION();

    // ViewEntity<Entity, RigidbodyComponent>([&](auto e, auto& rb){
    //   rb.ID = {};
    // });

    // ViewEntity<Entity, CharacterComponent>([&](auto e, auto& cc){
    //   cc.Body = {};
    // });    

    m_Physics3D.Init();
    m_Physics3D.StartSimulation();

    UE_CORE_INFO("[OnRuntimeStart]: scene={} physics_sys={}",
                 (const void*)this, (const void*)&m_Physics3D.System());

    auto& sys = m_Physics3D.System();
    auto& bi  = m_Physics3D.Bodies();

    // Characters first (so we don't create a rigidbody for them)
    ViewEntity<Entity, CharacterComponent>(
        [&](auto e, auto &cc) {
          auto &tr = e.template GetComponent<TransformComponent>();

          if (cc.Body.IsInvalid()) {
            BuildCharacterForEntity(e, tr, cc);   // creates a kinematic capsule (your helper)
          }

          if (!cc.Body.IsInvalid()) {
            bi.SetPositionAndRotation(
                cc.Body,
                JPH::RVec3(tr.Translation.x, tr.Translation.y, tr.Translation.z),
                ToJoltQuat(glm::quat(tr.Rotation)),
                JPH::EActivation::Activate
            );

            // initialize character runtime state
            cc.SpawnSynced = true;
            cc.VerticalVel = 0.0f;
            cc.Grounded    = false;
            // cc.WishMove    = {0.0f};
            cc.Jump = false;
          } else {
	    UE_CORE_WARN("[OnRuntimeStart]: no valid Body in CharacterCompoenet");
	  }
        });

    // 2) Build colliders if needed
    ViewEntity<Entity, BoxShapeComponent>([&](auto e, auto &c) {
      if (!c.Shape || c.Dirty) {
	UE_CORE_INFO("[OnRuntimeStart]: BoxShapeComponent for {} dirty", e.GetName());
        c.Shape = BuildBox(c.HalfExtents);
        c.Dirty = false;
      }
      
    });
    ViewEntity<Entity, SphereShapeComponent>([&](auto e, auto &c) {
      if (!c.Shape || c.Dirty) {
	UE_CORE_INFO("[OnRuntimeStart]: SphereShapeComponent for {} dirty", e.GetName());
        c.Shape = BuildSphere(c.Radius);
        c.Dirty = false;
      }      
    });

    // Rigidbodies (skip if entity has a CharacterComponent)
    ViewEntity<Entity, RigidbodyComponent>([&](auto e, auto &rb){
      if (e.template HasComponent<CharacterComponent>()) return;
      auto &tr = e.template GetComponent<TransformComponent>();
      if (rb.ID.IsInvalid()) {
	if (e.template HasComponent<BoxShapeComponent>())
	  BuildRigidBodyForEntity(e, tr, e.template GetComponent<BoxShapeComponent>());
	if (e.template HasComponent<SphereShapeComponent>())
	  BuildRigidBodyForEntity(e, tr, e.template GetComponent<SphereShapeComponent>());
      }

      if (!rb.ID.IsInvalid()) {
        m_Physics3D.Bodies().SetPositionAndRotation(
						    rb.ID,
						    JPH::RVec3(tr.Translation.x, tr.Translation.y, tr.Translation.z),
						    ToJoltQuat(glm::quat(tr.Rotation)),
						    JPH::EActivation::Activate
						    );
      }else {
	UE_CORE_WARN("[OnRuntimeStart]: no valid ID in RigidbodyComponent");
        }
    });

    ViewEntity<Entity, NativeScriptComponent>([=](auto entity, auto& nsc)
    {
      JPH::BodyInterface &body_interface = m_Physics3D.Bodies();

      if (!nsc.Instance) {
        UE_CORE_ASSERT(nsc.InstantiateScript, "NativeScriptComponent missing Bind()");
        nsc.Instance = nsc.InstantiateScript();
        // >>> set context first <<<
        nsc.Instance->m_Entity        = Entity{ entity, this };
        nsc.Instance->m_Scene         = this;
        nsc.Instance->m_BodyInterface = &body_interface;
        // now it's safe to enter user code
        nsc.Instance->OnCreate();
      } else {
        // keep context up to date for already-instantiated scripts
        nsc.Instance->m_Entity        = Entity{ entity, this };
        nsc.Instance->m_Scene         = this;
        nsc.Instance->m_BodyInterface = &body_interface;
      }

    });
  }
  

  // ------------------------------
  // Scene::OnRuntimeStop
  // ------------------------------
  void Scene::OnRuntimeStop()
  {
    UE_PROFILE_FUNCTION();

    auto &sys = m_Physics3D.System();
    auto &bi  = m_Physics3D.Bodies();

    // Stop further stepping
    // m_RuntimeRunning = false;

    // 1) Collect body ids (Rigidbody + Character)
    std::vector<JPH::BodyID> ids;
    ids.reserve(64);

    ViewEntity<Entity, RigidbodyComponent>([&](auto, auto &rb) {
      if (!rb.ID.IsInvalid()) ids.push_back(rb.ID);
    });

    ViewEntity<Entity, CharacterComponent>([&](auto, auto &cc) {
      if (!cc.Body.IsInvalid()) ids.push_back(cc.Body);
    });

    // Deduplicate
    std::sort(ids.begin(), ids.end(), [](const JPH::BodyID &a, const JPH::BodyID &b) {
      return a.GetIndexAndSequenceNumber() < b.GetIndexAndSequenceNumber();
    });
    ids.erase(std::unique(ids.begin(), ids.end(), [](const JPH::BodyID &a, const JPH::BodyID &b) {
      return a.GetIndexAndSequenceNumber() == b.GetIndexAndSequenceNumber();
    }), ids.end());

    // Filter out obviously bogus ids
    auto looks_valid = [](const JPH::BodyID &id) -> bool {
      return !id.IsInvalid() && id.GetIndex() != 0xFFFFFF;
    };
    ids.erase(std::remove_if(ids.begin(), ids.end(),
                             [&](const JPH::BodyID &id){ return !looks_valid(id); }),
              ids.end());

    UE_CORE_INFO("[Stop] Collected {} unique valid body ids", (int)ids.size());
    for (auto &b : ids)
      UE_CORE_INFO("  id idx+seq = {}", (uint32_t)b.GetIndexAndSequenceNumber());

    // If nothing to do, just invalidate components and return
    if (ids.empty()) {
      ViewEntity<Entity, RigidbodyComponent>([&](auto, auto &rb){ rb.ID = {}; });
      ViewEntity<Entity, CharacterComponent>([&](auto, auto &cc){ cc.Body = {}; });
      return;
    }

    // 2) Remove only if currently added
    for (const JPH::BodyID &id : ids) {
      if (bi.IsAdded(id)) {
	bi.RemoveBody(id);
      }
    }

    // 3) Destroy only those that still exist (verify via lock)
    std::vector<JPH::BodyID> to_destroy;
    to_destroy.reserve(ids.size());

    const JPH::BodyLockInterfaceNoLock &bli = sys.GetBodyLockInterfaceNoLock();
    for (const JPH::BodyID &id : ids) {
      JPH::BodyLockRead lock(bli, id);
      if (lock.Succeeded()) {
	to_destroy.push_back(id);
      }
    }

    if (!to_destroy.empty()) {
      bi.DestroyBodies(to_destroy.data(), (int)to_destroy.size());
    }

    // 4) Invalidate component handles (prevents double-destroy later)
    ViewEntity<Entity, RigidbodyComponent>([&](auto, auto &rb){ rb.ID = {}; });
    ViewEntity<Entity, CharacterComponent>([&](auto, auto &cc){ cc.Body = {}; });
  }

  
  // ------------------------------
  // Scene::PhysicsUpdate
  // ------------------------------
  void Scene::PhysicsUpdate(float dt)
  {
    UE_PROFILE_FUNCTION();
    UE_CORE_WARN("[PhysicsUpdate]");
    auto& sys = m_Physics3D.System();
    auto& bi  = m_Physics3D.Bodies();

    UE_CORE_INFO("[PhysicsUpdate]: scene={} physics_sys={}",
                 (const void*)this, (const void*)&sys);
   // --- Drive kinematic characters from input (WishMove / JumpRequested) ---
    ViewEntity<Entity, CharacterComponent>([&](auto e, auto &cc) {
      if (cc.Body.IsInvalid()) {
	UE_CORE_ERROR("[PhysicsUpdate]: CharacterComponent Body for {} with id={} is invalid", e.GetName(), (uint)cc.Body.GetIndexAndSequenceNumber());
        return;
      }
      if (!bi.IsAdded(cc.Body)) {
        UE_CORE_ERROR("[PhysicsUpdate]: CharacterComponent for {} with id={} is not added", e.GetName(), (uint)cc.Body.GetIndexAndSequenceNumber());
        return;
      }

      // Current pose
      const JPH::RMat44 xf = bi.GetWorldTransform(cc.Body);

      // Horizontal velocity from input in world space
      JPH::Vec3 wish = ToJoltVec3(cc.WishMove);            // set by your script each frame
      const float wl = wish.Length();
      const JPH::Vec3 hvel = wl > 0.0f ? (wish / wl) * cc.MaxSpeed : JPH::Vec3::sZero();

      // Ground ray: 2 m down from COM (R* types because NarrowPhase uses double-precision casts)
      JPH::RRayCast ray(xf.GetTranslation(), JPH::RVec3(0.0, -2.0, 0.0));
      JPH::RayCastResult hit;

      // Use same filters the world was built with: CHARACTER collides with NON_MOVING, etc.
      JPH::DefaultBroadPhaseLayerFilter bp(m_Physics3D.GetOVBPFilter(), Layers::CHARACTER);
      JPH::DefaultObjectLayerFilter     of(m_Physics3D.GetPairFilter(),  Layers::CHARACTER);

      const bool ray_hit = sys.GetNarrowPhaseQuery().CastRay(ray, hit, bp, of);
      cc.Grounded = ray_hit && hit.mFraction < 1.0f;

      // Vertical motion
      if (cc.Grounded) {
	if (cc.Jump) {
	  cc.VerticalVel   = cc.JumpImpulse;   // e.g. 5–8 m/s
	  cc.Jump = false;
	} else {
	  if (std::abs(cc.VerticalVel) < 0.01f) cc.VerticalVel = 0.0f; // kill tiny drift
	}
      } else {
	cc.VerticalVel -= cc.Gravity * dt;       // e.g. 9.81
      }

      // Compose velocity and move kinematic
      const JPH::RVec3 vel(hvel.GetX(), cc.VerticalVel, hvel.GetZ());
      const JPH::RVec3 target_pos = xf.GetTranslation() + vel * dt;

      UE_CORE_INFO("[CHARAC POS PHYSICS]: {},{},{}", target_pos.GetX(), target_pos.GetY(), target_pos.GetZ());

      bi.MoveKinematic(cc.Body, target_pos, xf.GetRotation().GetQuaternion(), dt);
    });

    // --- Step the physics world ---
    // Use your wrapper; replace this line with your actual step call if different.
    m_Physics3D.Step(dt);

    auto &bli = sys.GetBodyLockInterface();    

    // --- Copy back transforms from physics (ONLY for entities with valid bodies) ---
    // ViewEntity<Entity, RigidbodyComponent>([&](auto e, auto &rb) {
    //   if (rb.ID.IsInvalid())
    //     return;

    //   JPH::BodyLockRead lock(bli, rb.ID);
    //   if (!lock.Succeeded()) {
    // 	UE_CORE_WARN("[PhysUpdate] Rigidbody BodyLockRead failed id={} name={}",
    //                      (uint)rb.ID.GetIndexAndSequenceNumber(),
    // 		     e.GetName());
    //     return;
    //   }

    //   UE_CORE_WARN("[PHYSICS UPDATE RIGIDBODY COMP]");
      
    //   auto &tr = e.template GetComponent<TransformComponent>();
    //   const JPH::Body &body = lock.GetBody();
      
    //   if (rb.Type == BodyType::Dynamic) {
    // 	UE_CORE_INFO("[Dynamic]");
    // 	// physics -> ECS
    // 	const JPH::RMat44 xf = body.GetWorldTransform();
    //     // const JPH::RVec3 p = bi.GetCenterOfMassPosition(rb.ID);
    // 	const JPH::RVec3 p = xf.GetTranslation();
    //     const JPH::Quat q = bi.GetRotation(rb.ID);
    // 	UE_CORE_WARN("JPH {} {} {}", p.GetX(), p.GetY(), p.GetZ());
    //     tr.Translation = {(float)p.GetX(), (float)p.GetY(), (float)p.GetZ()};
    // 	UE_CORE_WARN("TRANS {} {} {}", tr.Translation.x, tr.Translation.y, tr.Translation.z);
    // 	tr.Rotation    = glm::eulerAngles(ToGlmQuat(q)); // if you store Euler
    //   } else if (rb.Type == BodyType::Kinematic) {
    //     // ECS -> physics
    // 	UE_CORE_INFO("[Kinematic]");
    // 	bi.SetPositionAndRotation(
    // 				  rb.ID,
    // 				  JPH::RVec3(tr.Translation.x, tr.Translation.y, tr.Translation.z),
    // 				  ToJoltQuat(glm::quat(tr.Rotation)),
    // 				  JPH::EActivation::Activate
    // 				  );
    //   } else {
    // 	UE_CORE_INFO("[Static]");
    //   }
    //   // Static: no per-frame push

    //   UE_CORE_WARN("[PHYSICS UPDATE RIGIDBODY COMP DONE]");
    // });

    ViewEntity<Entity, RigidbodyComponent>([&](auto e, auto &rb) {
      if (rb.ID.IsInvalid()){
	UE_CORE_ERROR("[PhysicsUpdate]: RigidbodyComponent ID for {} with id={} is invalid", e.GetName(), (uint)rb.ID.GetIndexAndSequenceNumber());
        return;
      }

      // JPH::BodyLockRead lock(bli, rb.ID);
      // if (!lock.Succeeded()) {
      // 	UE_CORE_WARN("[PhysicsUpdate]: RigidbodyComponent BodyLockRead failed id={} name={}",
      //                    (uint)rb.ID.GetIndexAndSequenceNumber(),
      // 		     e.GetName());
      //   return;
      // }

      // const JPH::Body &body = lock.GetBody();
      auto &tr = e.template GetComponent<TransformComponent>();

      if (rb.Type == BodyType::Dynamic) {
        // physics -> ECS
        // const JPH::RMat44 xf = body.GetWorldTransform();
	const JPH::RMat44 xf = bi.GetWorldTransform(rb.ID);
	const JPH::RVec3 p = xf.GetTranslation();        
	// const JPH::RVec3 p = bi.GetCenterOfMassPosition(rb.ID);
        const JPH::Quat q = bi.GetRotation(rb.ID);
	UE_CORE_INFO("[PhysicsUpdate]: RigidbodyComponent JPH {} {} {}", p.GetX(), p.GetY(), p.GetZ());
        tr.Translation = {(float)p.GetX(), (float)p.GetY(), (float)p.GetZ()};
	UE_CORE_INFO("[PhysicsUpdate]: RigidbodyComponent TRANS {} {} {}", tr.Translation.x, tr.Translation.y, tr.Translation.z);
	tr.Rotation    = glm::eulerAngles(ToGlmQuat(q)); // if you store Euler
      } else if (rb.Type == BodyType::Kinematic) {
	// ECS -> physics
	bi.SetPositionAndRotation(
				  rb.ID,
				  JPH::RVec3(tr.Translation.x, tr.Translation.y, tr.Translation.z),
				  ToJoltQuat(glm::quat(tr.Rotation)),
				  JPH::EActivation::Activate
				  );
      }
      // Static: no per-frame push
    });
    
    ViewEntity<Entity, CharacterComponent>([&](auto e, auto &cc) {
      if (cc.Body.IsInvalid()){
	UE_CORE_ERROR("[PhysicsUpdate]: CharacterComponent Body for {} with id={} is invalid", e.GetName(), (uint)cc.Body.GetIndexAndSequenceNumber());
        return;
      }

      // JPH::BodyLockRead lock(bli, cc.Body);
      // if (!lock.Succeeded()) {
      // 	UE_CORE_WARN("[PhysicsUpdate]: CharacterComponent BodyLockRead failed id={} name={}",
      // 		     (uint)cc.Body.GetIndexAndSequenceNumber(),
      // 		     e.GetName());
      // 	return;
      // }
      
      auto &tr = e.template GetComponent<TransformComponent>();

      const JPH::RMat44 xf = bi.GetWorldTransform(cc.Body);
      const JPH::RVec3  p  = xf.GetTranslation();
      tr.Translation = { (float)p.GetX(), (float)p.GetY(), (float)p.GetZ() };

      const JPH::Quat q = xf.GetRotation().GetQuaternion();
      tr.Rotation = glm::eulerAngles(ToGlmQuat(q)); 
    });
    FlushEntityDestruction();
  }




  void Scene::OnUpdateRuntime(Timestep ts, int& mouseX, int& mouseY, glm::vec2& viewportSize){
    UE_PROFILE_FUNCTION();
    m_Framebuffer->Bind();
    // Clear our entity ID attachment to -1
    m_Framebuffer->ClearAttachment(1, -1);
    RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    RenderCommand::Clear();

    // Update scripts
    {
      m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
      {
	JPH::BodyInterface &body_interface = m_Physics3D.Bodies();
	// TODO: Move to Scene::OnScenePlay
        if (!nsc.Instance) {
	  UE_CORE_ASSERT(nsc.InstantiateScript, "NativeScriptComponent missing Bind()");
	  nsc.Instance = nsc.InstantiateScript();
	  // >>> set context first <<<
	  nsc.Instance->m_Entity        = Entity{ entity, this };
	  nsc.Instance->m_Scene         = this;
	  nsc.Instance->m_BodyInterface = &body_interface;
	  // now it's safe to enter user code
	  nsc.Instance->OnCreate();
	}else {
	  // keep context up to date for already-instantiated scripts
	  nsc.Instance->m_Entity        = Entity{ entity, this };
	  nsc.Instance->m_Scene         = this;
	  nsc.Instance->m_BodyInterface = &body_interface;
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

    ViewEntity<Entity, TransformComponent>([this](auto e, auto &tr) {
      // somewhere in your inspector apply path
      if (e.template HasComponent<BoxShapeComponent>()) {
	auto& c = e.template GetComponent<BoxShapeComponent>();
	if (c.Dirty) {
	  c.Shape = BuildBox(c.HalfExtents);
	  c.Dirty = false;

	  // Rebuild body if present
	  if (e.template HasComponent<RigidbodyComponent>()) {
            auto& rb = e.template GetComponent<RigidbodyComponent>();
            auto& bi = m_Physics3D.Bodies();
            if (!rb.ID.IsInvalid()) {
              bi.RemoveBody(rb.ID);
              bi.DestroyBody(rb.ID);
              rb.ID = {};
            }
            BuildRigidBodyForEntity(e, tr, c);
	  }
	}
      }
    });

    // Apply path (editor or play)
    ViewEntity<Entity, CharacterComponent>([&](auto e, auto& c){
      if (!c.Dirty) return;
      c.Shape = BuildCapsule(c.HalfHeight, c.Radius);
      c.Dirty = false;

      // Recreate body if it exists
      if (c.Body.IsInvalid()) {
        auto& tr = e.template GetComponent<TransformComponent>();
        BuildCharacterForEntity(e, tr, c);
      } else {
        auto& bi = m_Physics3D.Bodies();
        bi.RemoveBody(c.Body);
        bi.DestroyBody(c.Body);
        c.Body = {};
        auto& tr = e.template GetComponent<TransformComponent>();
        BuildCharacterForEntity(e, tr, c);
      }
    });


		
    if (mainCamera)
      {			
			
	// Renderer3D::BeginCamera(*mainCamera, tc);
	Renderer3D::BeginCamera(*mainCamera);

	Renderer3D::RenderLight({5.5f, 5.0f, 0.3f });

	GroupEntity<ModelComponent>([this](auto entity, auto& comp, auto& transform, auto id){
	  Renderer3D::DrawModel(comp.ModelData, transform.GetTransform(), glm::vec3(1.0f),(int)id);
	});

	auto CubeGroup = m_Registry.group<CubeComponent>(entt::get<TransformComponent>);
	for (auto entity : CubeGroup) {
	  auto [transform, CubeComp] = CubeGroup.get<TransformComponent, CubeComponent>(entity);
				
	  Renderer3D::DrawCube(transform.GetTransform(), CubeComp.Color, (int)entity);
	}

	auto boxShapeGroup = m_Registry.group<BoxShapeComponent>(entt::get<TransformComponent>);
	for (auto entity : boxShapeGroup) {
	  auto [transform, boxComp] = boxShapeGroup.get<TransformComponent, BoxShapeComponent>(entity);				
				
	  const auto& box = boxComp.Shape->GetLocalBounds();

	  if(ShowBoxesPlay)
	    Renderer3D::DrawWireCube({transform.Translation.x, transform.Translation.y, transform.Translation.z}, 
				     {box.GetSize().GetX(), box.GetSize().GetY(), box.GetSize().GetZ()}, 
				     {0,1,0}, 0.5f);			
				
	}

	auto sphereShapeGroup = m_Registry.group<SphereShapeComponent>(entt::get<TransformComponent>);
	for (auto entity : sphereShapeGroup) {
	  auto [transform, sphereComp] = sphereShapeGroup.get<TransformComponent, SphereShapeComponent>(entity);				
				
	  const auto& box = sphereComp.Shape->GetLocalBounds();

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

    FlushEntityDestruction();
  }

  void Scene::OnUpdateEditor(Timestep ts, EditorCamera &camera, int &mouseX,
                             int &mouseY, glm::vec2 &viewportSize) {
    UE_CORE_WARN("[OnUpdateEditor]");
    UE_PROFILE_FUNCTION();
    m_Framebuffer->Bind();
    // Clear our entity ID attachment to -1
    m_Framebuffer->ClearAttachment(1, -1);
    RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    RenderCommand::Clear();

    // m_Physics3D.Simulate(ts);
    // glEnable(GL_DEPTH_TEST);
    Renderer3D::BeginCamera(camera);
    //temp
    Renderer3D::RenderLight({5.5f, 5.0f, 0.3f});

    ViewEntity<Entity, TransformComponent>([this](auto e, auto &c) {
      // somewhere in your inspector apply path
      if (e.template HasComponent<BoxShapeComponent>()) {
	auto& c = e.template GetComponent<BoxShapeComponent>();
	if (c.Dirty) {
	  c.Shape = BuildBox(c.HalfExtents);
	  c.Dirty = false;

	  // Rebuild body if present
	  // if (e.template HasComponent<RigidbodyComponent>()) {
          //   auto& rb = e.template GetComponent<RigidbodyComponent>();
          //   auto& bi = m_Physics3D.Bodies();
          //   if (!rb.ID.IsInvalid()) {
          //     bi.RemoveBody(rb.ID);
          //     bi.DestroyBody(rb.ID);
          //     rb.ID = {};
          //   }
          //   auto& tr = e.template GetComponent<TransformComponent>();
          //   BuildRigidBodyForEntity(e, tr, c);
	  // }
	}
      }
    });

    ViewEntity<Entity, CharacterComponent>([&](auto e, auto& c){
      if (!c.Dirty) return;
      c.Shape = BuildCapsule(c.HalfHeight, c.Radius);
      c.Dirty = false;

      // Recreate body if it exists
      // if (c.Body.IsInvalid()) {
      //   auto& tr = e.template GetComponent<TransformComponent>();
      //   BuildCharacterForEntity(e, tr, c);
      // } else {
      //   auto& bi = m_Physics3D.Bodies();
      //   bi.RemoveBody(c.Body);
      //   bi.DestroyBody(c.Body);
      //   c.Body = {};
      //   auto& tr = e.template GetComponent<TransformComponent>();
      //   BuildCharacterForEntity(e, tr, c);
      // }
    });

    GroupEntity<ModelComponent>([this] (auto entity, auto& comp, auto& transform, auto id){
      // UE_CORE_WARN("Model entity: {}", (int)id);
      Renderer3D::DrawModel(comp.ModelData, transform.GetTransform(), glm::vec3(1.0f), 1.0f,(int)id);	
    });

    GroupEntity<CubeComponent>([this] (auto entity, auto& comp, auto& transform, auto id){						
      // UE_CORE_WARN("Cube entity: {}", (int)entity);
      Renderer3D::DrawCube(transform.GetTransform(), comp.Color, 1.0f, (int)id);
    });

    GroupEntity<CameraComponent>([this] (auto entity, auto& comp, auto& transform, auto id){						
      comp.Camera.m_Position = &transform.Translation;
      // CamComp.Camera.m_Rotation2 = &transform.Rotation;
      if(ShowCams)
	Renderer3D::DrawCube(transform.GetTransform(), {1,0,0}, 1.0f, (int)id);
      // Renderer3D::DrawCameraFrustum(comp.Camera);

      // UE_CORE_WARN("Camera entity: {}", (int)id);
    });

    GroupEntity<BoxShapeComponent>([this] (auto entity, auto& comp, auto& transform, auto id){      
			
      const auto& box = comp.Shape->GetLocalBounds();

      if(ShowBoxes)
	Renderer3D::DrawWireCube({transform.Translation.x, transform.Translation.y, transform.Translation.z}, 
				 {box.GetSize().GetX(), box.GetSize().GetY(), box.GetSize().GetZ()}, 
				 {0,1,0}, 0.5f);	
				
      Renderer3D::SetEntity((int)entity);

    });

    GroupEntity<SphereShapeComponent>([this](auto entity, auto &comp,
                                             auto &transform, auto id) {			
			
      const auto& box = comp.Shape->GetLocalBounds();

      if(ShowBoxes)
	Renderer3D::DrawWireSphere({transform.Translation.x, transform.Translation.y, transform.Translation.z}, 
				   {box.GetSize().GetX(), box.GetSize().GetY(), box.GetSize().GetZ()}, 
				   {0,0,1}, 0.5f);
    });

    GroupEntity<CharacterComponent>(
        [this](auto entity, auto &comp, auto &transform, auto id) {
          const auto &box = comp.Shape->GetLocalBounds();

          if (ShowBoxes)
	    Renderer3D::DrawWireSphere(transform.Translation, {box.GetSize().GetX(), box.GetSize().GetY(), box.GetSize().GetZ()}, 
				       {0.0f,0.5f,1.0f}, 0.5f);
      
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

    FlushEntityDestruction();
  }

  void Scene::DrawScreen(Ref<Framebuffer>& buffer, EditorCamera& camera){		
    Renderer2D::BeginCamera(camera);
    Renderer2D::DrawScreen(buffer);
    Renderer2D::EndCamera();
  }

  void Scene::OnViewportResize(uint32_t width, uint32_t height)
  {
    UE_PROFILE_FUNCTION();
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

  void Scene::FlushEntityDestruction()
  {
    if (m_DestroyQueue.empty()) return;    
    for (auto e : m_DestroyQueue)
      if (m_Registry.valid(e))
	m_Registry.destroy(e);

    m_DestroyQueue.clear();
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

  template <>
  void
  Scene::OnComponentAdded<RigidbodyComponent>(Entity entity,
                                              RigidbodyComponent &component) {
    if (entity.HasComponent<CharacterComponent>()) {
      UE_CORE_WARN("Entity has both Character and Rigidbody. Removing Character.");
      entity.RemoveComponent<CharacterComponent>();
    }
    // If a collider is already present, create the body now
    auto& tr = entity.GetComponent<TransformComponent>();
    // if (entity.HasComponent<BoxShapeComponent>())    BuildRigidBodyForEntity(entity, tr, entity.GetComponent<BoxShapeComponent>());
    // if (entity.HasComponent<SphereShapeComponent>()) BuildRigidBodyForEntity(entity, tr, entity.GetComponent<SphereShapeComponent>());
  }

  template<>
    void  Scene::OnComponentAdded<BoxShapeComponent>(Entity entity, BoxShapeComponent& component)
  {
    component.Shape = BuildBox(component.HalfExtents);
    component.Dirty = false;
    if (entity.HasComponent<RigidbodyComponent>()) {
        auto& tr = entity.GetComponent<TransformComponent>();
        BuildRigidBodyForEntity(entity, tr, component);
    }    
  }

  template<>
    void  Scene::OnComponentAdded<SphereShapeComponent>(Entity entity, SphereShapeComponent& component)
  {
    component.Shape = BuildSphere(component.Radius);
    component.Dirty = false;
    if (entity.HasComponent<RigidbodyComponent>()) {
        auto& tr = entity.GetComponent<TransformComponent>();
        BuildRigidBodyForEntity(entity, tr, component);
    }
  }

  template<>
    void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
  {
    
  }

  template <>
  void
  Scene::OnComponentAdded<CharacterComponent>(Entity entity,
                                              CharacterComponent &component) {
    if (entity.HasComponent<RigidbodyComponent>()) {
      UE_CORE_WARN("Entity has both Character and Rigidbody. Removing Rigidbody.");
      entity.RemoveComponent<RigidbodyComponent>();
    }
    // Build capsule shape once
    component.Shape = BuildCapsule(component.HalfHeight, component.Radius);
    component.Dirty = false;

    // Create the kinematic body
    auto& tr = entity.GetComponent<TransformComponent>();
    // BuildCharacterForEntity(entity, tr, component);
  }

  template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
  {
    if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
      component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
  }
}

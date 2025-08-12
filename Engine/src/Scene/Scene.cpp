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
    if (!sc.Shape) return;

    auto& rb = e.GetComponent<RigidbodyComponent>();

    JPH::EMotionType motion = JPH::EMotionType::Dynamic;
    JPH::ObjectLayer layer  = Layers::MOVING;
    switch (rb.Type) {
    case BodyType::Static:    motion = JPH::EMotionType::Static;    layer = Layers::NON_MOVING; break;
    case BodyType::Kinematic: motion = JPH::EMotionType::Kinematic; layer = Layers::MOVING;     break;
    case BodyType::Dynamic:   motion = JPH::EMotionType::Dynamic;   layer = Layers::MOVING;     break;
    }

    const JPH::RVec3 pos(tr.Translation.x, tr.Translation.y, tr.Translation.z);
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

    auto& bi = m_Physics3D.Bodies();
    rb.ID = bi.CreateAndAddBody(bcs, JPH::EActivation::Activate);

    if (rb.Type == BodyType::Dynamic && rb.Mass > 0.0f) {
      // Optional: set mass properties (or leave Jolt to compute from shape density)
      bi.SetLinearVelocity(rb.ID, JPH::Vec3(0,0,0));
      bi.SetAngularVelocity(rb.ID, JPH::Vec3(0,0,0));
    }
  }

  template <typename TChar>
  void Scene::BuildCharacterForEntity(Entity e, TransformComponent& tr, TChar& cc) {
    if (!cc.Shape) return;

    const JPH::RVec3 pos(tr.Translation.x, tr.Translation.y, tr.Translation.z);
    UE_CORE_WARN("POS {} {} {}", pos.GetX(), pos.GetY(), pos.GetZ());

    // If you store Euler in Transform:
    const glm::quat gq = glm::quat(tr.Rotation);
    const JPH::Quat rot = ToJoltQuat(gq);

    

    JPH::BodyCreationSettings bcs(
				  cc.Shape,
				  pos,
				  rot,
				  JPH::EMotionType::Kinematic,
				  Layers::MOVING // your object layer
				  );
    bcs.mFriction      = 0.8f;
    bcs.mRestitution   = 0.0f;
    bcs.mMotionQuality = cc.EnableCCD ? JPH::EMotionQuality::LinearCast
      : JPH::EMotionQuality::Discrete;
    bcs.mUserData      = (JPH::uint64)(entt::entity)e;

    auto& bi = m_Physics3D.Bodies();
    cc.Body = bi.CreateAndAddBody(bcs, JPH::EActivation::Activate);
  }



  // ------------------------------
  // Scene::OnRuntimeStart
  // ------------------------------
  void Scene::OnRuntimeStart() {
    UE_PROFILE_FUNCTION();

    // 0) Bring physics up
    m_Physics3D.Init();
    m_Physics3D.StartSimulation();

    // 1) Instantiate scripts first so their OnCreate() can set final transforms
    m_Registry.view<NativeScriptComponent>().each([&](auto entt_handle, auto &nsc){
      if (!nsc.Instance) {
	nsc.Instance = nsc.InstantiateScript();
        nsc.Instance->m_Entity = Entity(entt_handle, this);
        nsc.Instance->m_Scene = this;
	nsc.Instance->m_BodyInterface = &m_Physics3D.Bodies();
      }
      nsc.Instance->OnCreate();
    });

    // 2) Build colliders if needed
    ViewEntity<Entity, BoxShapeComponent>([&](auto e, auto &c){
      if (!c.Shape || c.Dirty) { c.Shape = BuildBox(c.HalfExtents); c.Dirty = false; }
    });
    ViewEntity<Entity, SphereShapeComponent>([&](auto e, auto &c){
      if (!c.Shape || c.Dirty) { c.Shape = BuildSphere(c.Radius);   c.Dirty = false; }
    });

    // 3) Create rigidbodies (skip entities that also have CharacterComponent)
    ViewEntity<Entity, RigidbodyComponent>([&](auto e, auto &rb){
      if (e.template HasComponent<CharacterComponent>()) return;
      if (rb.ID.IsInvalid()) {
	auto &tr = e.template GetComponent<TransformComponent>();
	if (e.template HasComponent<BoxShapeComponent>())
	  BuildRigidBodyForEntity(e, tr, e.template GetComponent<BoxShapeComponent>());
	if (e.template HasComponent<SphereShapeComponent>())
	  BuildRigidBodyForEntity(e, tr, e.template GetComponent<SphereShapeComponent>());
      }
    });

    // 4) Create characters and teleport them ONCE to authored Transform
    auto &bi = m_Physics3D.Bodies();
    ViewEntity<Entity, CharacterComponent>([&](auto e, auto &cc) {
      auto &tr = e.template GetComponent<TransformComponent>();
      if (!cc.Shape || cc.Dirty) { cc.Shape = BuildCapsule(cc.HalfHeight, cc.Radius); cc.Dirty = false; }
      if (cc.Body.IsInvalid())   BuildCharacterForEntity(e, tr, cc);
      if (cc.Body.IsInvalid())   return;

      bi.SetPositionAndRotation(
				cc.Body,
				JPH::RVec3(tr.Translation.x, tr.Translation.y, tr.Translation.z),
				ToJoltQuat(glm::quat(tr.Rotation)),
				JPH::EActivation::Activate
				);
      cc.LastY = tr.Translation.y;
      cc.SpawnSynced = true;     // pre-step won’t resync from ECS
    });
  }


  // ------------------------------
  // Scene::OnRuntimeStop
  // ------------------------------
  void Scene::OnRuntimeStop() {
    UE_PROFILE_FUNCTION();

    auto &sys = m_Physics3D.System();
    auto &bi  = m_Physics3D.Bodies();

    // Non-copyable interface: take a reference
    const JPH::BodyLockInterface &bli = sys.GetBodyLockInterface();

    // Characters
    ViewEntity<Entity, CharacterComponent>([&](auto, auto &cc){
      if (cc.Body.IsInvalid()) return;

      JPH::BodyLockWrite lock(bli, cc.Body);
      if (!lock.Succeeded()) { cc.Body = {}; return; }

      if (bi.IsAdded(cc.Body)) bi.RemoveBody(cc.Body);
      bi.DestroyBody(cc.Body);
      cc.Body = {};
      cc.SpawnSynced = false;
    });

    // Rigidbodies
    ViewEntity<Entity, RigidbodyComponent>([&](auto, auto &rb){
      if (rb.ID.IsInvalid()) return;

      JPH::BodyLockWrite lock(bli, rb.ID);
      if (!lock.Succeeded()) { rb.ID = {}; return; }

      if (bi.IsAdded(rb.ID)) bi.RemoveBody(rb.ID);
      bi.DestroyBody(rb.ID);
      rb.ID = {};
    });

    // (optional) stop/shutdown physics here if your wrapper supports it
    // m_Physics3D.StopSimulation();
    // m_Physics3D.Shutdown();
  }



  
  // ------------------------------
  // Scene::PhysicsUpdate
  // ------------------------------
  void Scene::PhysicsUpdate(float dt) {
    UE_PROFILE_FUNCTION();
    auto &bi = m_Physics3D.Bodies();

    // === PRE-STEP: Characters ===
    ViewEntity<Entity, CharacterComponent>([&](auto e, auto &cc){
      if (cc.Body.IsInvalid()) return;
      auto &tr = e.template GetComponent<TransformComponent>();

      // One-time spawn sync in case this entity was duplicated/teleported this frame
      if (!cc.SpawnSynced) {
	bi.SetPositionAndRotation(
				  cc.Body,
				  JPH::RVec3(tr.Translation.x, tr.Translation.y, tr.Translation.z),
				  ToJoltQuat(glm::quat(tr.Rotation)),
				  JPH::EActivation::Activate
				  );
	cc.LastY = tr.Translation.y;
	cc.SpawnSynced = true;
      }

      // Vertical motion (simple)
      if (cc.Grounded && cc.Jump) { cc.VerticalVel = cc.JumpImpulse; cc.Grounded = false; }
      else                        { cc.VerticalVel += cc.Gravity * dt; }

      // Desired velocity from controller
      const glm::vec3 vel = cc.WishMove + glm::vec3(0.0f, cc.VerticalVel, 0.0f);

      // IMPORTANT: base on PHYSICS pose, not ECS (avoids zero snap)
      const JPH::RVec3 p0 = bi.GetCenterOfMassPosition(cc.Body);
      const JPH::RVec3 p1 = p0 + JPH::RVec3(vel.x * dt, vel.y * dt, vel.z * dt);

      const JPH::Quat rot = ToJoltQuat(glm::quat(tr.Rotation));
      bi.MoveKinematic(cc.Body, p1, rot, dt);

      cc.Jump = false; // consume
    });

    // === STEP the world ===
    m_Physics3D.Step(dt);

    // === POST-STEP: Rigidbodies ===
    ViewEntity<Entity, RigidbodyComponent>([&](auto e, auto &rb){
      if (rb.ID.IsInvalid()) return;
      auto &tr = e.template GetComponent<TransformComponent>();

      if (rb.Type == BodyType::Dynamic) {
	// physics -> ECS
	const JPH::RVec3 p = bi.GetCenterOfMassPosition(rb.ID);
        const JPH::Quat q = bi.GetRotation(rb.ID);
	UE_CORE_WARN("JPH {} {} {}", p.GetX(), p.GetY(), p.GetZ());
        tr.Translation = {(float)p.GetX(), (float)p.GetY(), (float)p.GetZ()};
	UE_CORE_WARN("TRANS {} {} {}", tr.Translation.x, tr.Translation.y, tr.Translation.z);
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

    // === POST-STEP: Characters -> ECS + grounded flag ===
    ViewEntity<Entity, CharacterComponent>([&](auto e, auto &cc){
      if (cc.Body.IsInvalid()) return;

      auto &tr = e.template GetComponent<TransformComponent>();
      const JPH::RVec3 p = bi.GetCenterOfMassPosition(cc.Body);
      const JPH::Quat  q = bi.GetRotation(cc.Body);

      const float newY = (float)p.GetY();
      const float dy   = newY - cc.LastY;

      tr.Translation = { (float)p.GetX(), newY, (float)p.GetZ() };
      tr.Rotation    = glm::eulerAngles(ToGlmQuat(q));

      // crude grounded inference
      if (cc.VerticalVel <= 0.0f && dy >= -1e-4f) { cc.Grounded = true; cc.VerticalVel = 0.0f; }
      else                                       { cc.Grounded = false; }

      cc.LastY = newY;
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
	if (!nsc.Instance)
	  {
	    nsc.Instance = nsc.InstantiateScript();
	    nsc.Instance->m_Entity = Entity{ entity, this };
	    nsc.Instance->m_Scene = this;
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

  void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera, int& mouseX, int& mouseY, glm::vec2& viewportSize)
  {
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
	  if (e.template HasComponent<RigidbodyComponent>()) {
            auto& rb = e.template GetComponent<RigidbodyComponent>();
            auto& bi = m_Physics3D.Bodies();
            if (!rb.ID.IsInvalid()) {
              bi.RemoveBody(rb.ID);
              bi.DestroyBody(rb.ID);
              rb.ID = {};
            }
            auto& tr = e.template GetComponent<TransformComponent>();
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

    GroupEntity<SphereShapeComponent>([this] (auto entity, auto& comp, auto& transform, auto id){			
			
      const auto& box = comp.Shape->GetLocalBounds();

      if(ShowBoxes)
	Renderer3D::DrawWireSphere({transform.Translation.x, transform.Translation.y, transform.Translation.z}, 
				   {box.GetSize().GetX(), box.GetSize().GetY(), box.GetSize().GetZ()}, 
				   {0,0,1}, 0.5f);	
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
    if (entity.HasComponent<BoxShapeComponent>())    BuildRigidBodyForEntity(entity, tr, entity.GetComponent<BoxShapeComponent>());
    if (entity.HasComponent<SphereShapeComponent>()) BuildRigidBodyForEntity(entity, tr, entity.GetComponent<SphereShapeComponent>());
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
    BuildCharacterForEntity(entity, tr, component);
  }

  template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
  {
    if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
      component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
  }
}

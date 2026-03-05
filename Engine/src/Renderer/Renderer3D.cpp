#include "Renderer/Renderer3D.h"
#include "Renderer/RenderCommand.h"
#include "Shader.h"
#include "glm/fwd.hpp"
#include "uepch.h"
#include <glm/gtc/type_ptr.hpp>
#include <ozz/base/maths/simd_math.h>
// #include "Core/Application.h"

// temp
// #include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace UE {
static Ref<Shader> m_Shader;
static Ref<Shader> m_ShaderSimple;
static Ref<Shader> m_LineShader;
static Ref<Shader> m_LightShader;
static Ref<Shader> m_SkyShader;
static glm::vec3 m_LightPos;
static Ref<Model> m_LightModel;
static Ref<Mesh> s_CubeMesh;
static Ref<Mesh> s_SphereMesh;

// TODO: remove
static GLuint lineVAO = 0, lineVBO = 0;

Ref<Mesh> GenerateSphereMesh(uint32_t sectorCount = 36,
                             uint32_t stackCount = 18) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  float x, y, z, xy;                  // vertex position
  float nx, ny, nz, lengthInv = 1.0f; // vertex normal
  float s, t;                         // vertex texCoord

  float sectorStep = 2 * glm::pi<float>() / sectorCount;
  float stackStep = glm::pi<float>() / stackCount;
  float sectorAngle, stackAngle;

  for (uint32_t i = 0; i <= stackCount; ++i) {
    stackAngle = glm::pi<float>() / 2 - i * stackStep; // from pi/2 to -pi/2
    xy = cos(stackAngle);                              // r * cos(u)
    z = sin(stackAngle);                               // r * sin(u)

    for (uint32_t j = 0; j <= sectorCount; ++j) {
      sectorAngle = j * sectorStep;

      x = xy * cos(sectorAngle); // x = r * cos(u) * cos(v)
      y = xy * sin(sectorAngle); // y = r * cos(u) * sin(v)
      nx = x;
      ny = y;
      nz = z;

      Vertex v;
      v.Position = glm::vec3(x, y, z);
      v.Normal = glm::normalize(glm::vec3(nx, ny, nz));
      v.TexCoords = glm::vec2((float)j / sectorCount, (float)i / stackCount);
      v.Tangent = glm::vec3(1, 0, 0);   // placeholder
      v.Bitangent = glm::vec3(0, 1, 0); // placeholder
      vertices.push_back(v);
    }
  }

  // index generation
  for (uint32_t i = 0; i < stackCount; ++i) {
    uint32_t k1 = i * (sectorCount + 1); // beginning of current stack
    uint32_t k2 = k1 + sectorCount + 1;  // beginning of next stack

    for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2) {
      if (i != 0)
        indices.insert(indices.end(), {k1, k2, k1 + 1});
      if (i != (stackCount - 1))
        indices.insert(indices.end(), {k1 + 1, k2, k2 + 1});
    }
  }

  std::vector<TextureMesh> noTextures;
  return CreateRef<Mesh>(vertices, indices, noTextures);
}

Ref<Mesh> GenerateCubeMesh() {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  glm::vec3 positions[] = {
      {-0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, 0.5f},
      {0.5f, 0.5f, 0.5f},    {-0.5f, 0.5f, 0.5f}, // front
      {-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
      {0.5f, 0.5f, -0.5f},   {0.5f, -0.5f, -0.5f}, // back
      {-0.5f, 0.5f, -0.5f},  {-0.5f, 0.5f, 0.5f},
      {0.5f, 0.5f, 0.5f},    {0.5f, 0.5f, -0.5f}, // top
      {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
      {0.5f, -0.5f, 0.5f},   {-0.5f, -0.5f, 0.5f}, // bottom
      {0.5f, -0.5f, -0.5f},  {0.5f, 0.5f, -0.5f},
      {0.5f, 0.5f, 0.5f},    {0.5f, -0.5f, 0.5f}, // right
      {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f},
      {-0.5f, 0.5f, 0.5f},   {-0.5f, 0.5f, -0.5f} // left
  };

  glm::vec3 normals[] = {{0, 0, 1},  {0, 0, -1}, {0, 1, 0},
                         {0, -1, 0}, {1, 0, 0},  {-1, 0, 0}};

  glm::vec2 uvs[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

  uint32_t faceIndices[] = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,
                            8,  9,  10, 10, 11, 8,  12, 13, 14, 14, 15, 12,
                            16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};

  for (int face = 0; face < 6; ++face) {
    for (int i = 0; i < 4; ++i) {
      Vertex v;
      v.Position = positions[face * 4 + i];
      v.Normal = normals[face];
      v.TexCoords = uvs[i];
      v.Tangent = glm::vec3(1, 0, 0);   // Placeholder
      v.Bitangent = glm::vec3(0, 1, 0); // Placeholder
      vertices.push_back(v);
    }
  }

  for (int i = 0; i < 36; ++i)
    indices.push_back(faceIndices[i]);

  std::vector<TextureMesh> noTextures; // Can be replaced later

  return CreateRef<Mesh>(vertices, indices, noTextures);
}

Ref<Shader> &Renderer3D::GetShader() { return m_Shader; }
void Renderer3D::Init() {
  UE_PROFILE_FUNCTION();

  m_Shader = Shader::Create("Data/Shaders/model.glsl");
  m_ShaderSimple = Shader::Create("Data/Shaders/Simplemodel.glsl");
  m_LineShader = Shader::Create("Data/Shaders/LineShader.glsl");
  m_LightShader = Shader::Create("Data/Shaders/BasicLight.glsl");
  m_SkyShader = Shader::Create("Data/Shaders/skybox.glsl");
  m_LightModel = CreateRef<Model>("Resources/cube.fbx");
  std::vector<std::string> faces = {
      "Resources/skybox/right.jpg", "Resources/skybox/left.jpg",
      "Resources/skybox/top.jpg",   "Resources/skybox/bottom.jpg",
      "Resources/skybox/front.jpg", "Resources/skybox/back.jpg"};

  // m_Skybox = Skybox::Create(faces);

  s_CubeMesh = GenerateCubeMesh();
  s_SphereMesh = GenerateSphereMesh();

  glGenVertexArrays(1, &lineVAO);
  glGenBuffers(1, &lineVBO);
  glBindVertexArray(lineVAO);
  glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr,
               GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
  glBindVertexArray(0);
}

void Renderer3D::Shutdown() {
  UE_PROFILE_FUNCTION();
  m_Shader.reset();
  m_ShaderSimple.reset();
  m_LineShader.reset();
  m_LightShader.reset();
  m_SkyShader.reset();
  m_LightModel.reset();
  s_CubeMesh.reset();
  s_SphereMesh.reset();
}

void Renderer3D::BeginCamera(const Camera &camera) {
  UE_PROFILE_FUNCTION();
  m_Shader->Bind();
  m_Shader->SetMat4("u_View", camera.GetViewMatrix());
  m_Shader->SetMat4("u_Projection", camera.GetProjectionMatrix());
  m_Shader->SetFloat3("u_ViewPos", camera.GetPosition());

  m_Shader->SetFloat3("u_LightPos", m_LightPos);
  m_LightShader->Bind();
  m_LightShader->SetMat4("u_View", camera.GetViewMatrix());
  m_LightShader->SetMat4("u_Projection", camera.GetProjectionMatrix());

  m_ShaderSimple->Bind();
  m_ShaderSimple->SetMat4("u_View", camera.GetViewMatrix());
  m_ShaderSimple->SetMat4("u_Projection", camera.GetProjectionMatrix());
  m_ShaderSimple->SetFloat3("u_ViewPos", camera.GetPosition());

  m_ShaderSimple->SetFloat3("u_LightPos", m_LightPos);

  m_LineShader->Bind();
  m_LineShader->SetMat4("u_View", camera.GetViewMatrix());
  m_LineShader->SetMat4("u_Projection", camera.GetProjectionMatrix());
}

void Renderer3D::EndCamera() { UE_PROFILE_FUNCTION(); }

void Renderer3D::RenderLight(const glm::vec3 &pos, const glm::vec4 &color) {
  UE_PROFILE_FUNCTION();
  m_LightShader->Bind();
  m_LightPos = pos;
  glm::mat4 imodel = glm::mat4(1.0f);
  imodel = glm::translate(imodel, pos);
  imodel = glm::rotate(imodel, glm::radians(0.0f), glm::vec3(0, 1, 0));
  imodel = glm::scale(imodel, glm::vec3(1));

  m_LightShader->SetMat4("u_Model", imodel);
  m_LightShader->SetFloat4("u_Color", color);

  m_LightModel->Draw(m_LightShader);
}

void Renderer3D::DrawSkybox(const Ref<Skybox> skybox, const Camera &camera) {

  m_SkyShader->Bind();
  skybox->Draw(m_SkyShader, camera.GetViewMatrix(),
               camera.GetProjectionMatrix());
}

void Renderer3D::SetEntity(int entityID) {
  m_Shader->Bind();
  m_Shader->SetInt("u_EntityID", entityID);
  m_Shader->Unbind();
}

void Renderer3D::DrawModel(const Ref<Model> &model, const glm::mat4 &transform,
                           const std::vector<ozz::math::Float4x4> *bones,
                           int entityID) {
  UE_PROFILE_FUNCTION();

  m_Shader->Bind();
  m_Shader->SetMat4("u_Model", transform);
  m_Shader->SetInt("u_EntityID", entityID);

  // ---- Upload bones if available ----
  if (bones && bones->size() > 0) {
    const int maxBones = 100;
    int count = std::min((int)bones->size(), maxBones);

    for (int i = 0; i < count; i++) {
      // glm::mat4 mat = glm::make_mat4(&(*bones)[i].cols[0].x);
      glm::mat4 mat(1.0f);

      alignas(16) float tmp[16];

      // Store 4 SIMD columns into memory
      ozz::math::StorePtr(bones->at(i).cols[0], tmp + 0);
      ozz::math::StorePtr(bones->at(i).cols[1], tmp + 4);
      ozz::math::StorePtr(bones->at(i).cols[2], tmp + 8);
      ozz::math::StorePtr(bones->at(i).cols[3], tmp + 12);

      mat = glm::make_mat4(tmp);

      std::string uniform = "u_FinalBonesMatrices[" + std::to_string(i) + "]";

      m_Shader->SetMat4(uniform, mat);
    }
  } else {
    // Identity fallback
    for (int i = 0; i < 100; i++) {
      std::string uniform = "u_FinalBonesMatrices[" + std::to_string(i) + "]";

      m_Shader->SetMat4(uniform, glm::mat4(1.0f));
    }
  }

  m_ShaderSimple->SetFloat3("u_Color", glm::vec3(1.0f));
  m_ShaderSimple->SetFloat("u_Transparancy", 1.0f);

  model->Draw(m_Shader);

  m_Shader->Unbind();
}

void Renderer3D::DrawCube(const glm::mat4 &transform, const glm::vec3 &color,
                          const float transparancy, int entityID) {
  UE_PROFILE_FUNCTION();
  m_ShaderSimple->Bind();
  m_ShaderSimple->SetMat4("u_Model", transform);
  m_ShaderSimple->SetFloat3("u_Color", color);
  m_ShaderSimple->SetInt("u_EntityID", entityID);
  m_ShaderSimple->SetFloat("u_Transparancy", transparancy);
  s_CubeMesh->Draw(m_ShaderSimple);
  m_ShaderSimple->Unbind();
}

Ref<Mesh> Renderer3D::GetCubeMesh() { return s_CubeMesh; }

void Renderer3D::DrawCube(const glm::vec3 &position, const glm::vec3 &size,
                          const glm::vec3 &color, const float transparancy) {

  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::scale(glm::mat4(1.0f), size);
  DrawCube(transform, color, transparancy);
}

void Renderer3D::DrawSphere(const glm::mat4 &transform, const glm::vec3 &color,
                            float transparancy, int entityID) {
  UE_PROFILE_FUNCTION();
  m_ShaderSimple->Bind();
  m_ShaderSimple->SetMat4("u_Model", transform);
  m_ShaderSimple->SetFloat3("u_Color", color);
  m_ShaderSimple->SetInt("u_EntityID", entityID);
  m_ShaderSimple->SetFloat("u_Transparancy", transparancy);
  s_SphereMesh->Draw(m_ShaderSimple);
  m_ShaderSimple->Unbind();
}

void Renderer3D::DrawSphere(const glm::vec3 &position, const glm::vec3 &scale,
                            const glm::vec3 &color, float transparancy) {

  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::scale(glm::mat4(1.0f), scale);
  DrawSphere(transform, color, transparancy);
}

void Renderer3D::DrawSphere(const glm::vec3 &position, float radius,
                            const glm::vec3 &color, float transparancy,
                            int entityID) {

  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::scale(glm::mat4(1.0f), glm::vec3(radius));
  DrawSphere(transform, color, transparancy, entityID);
}

void Renderer3D::DrawWireCube(const glm::vec3 &position, const glm::vec3 &size,
                              const glm::vec3 &color,
                              const float transparancy) {
  UE_PROFILE_FUNCTION();
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  DrawCube(position, size, color, transparancy);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer3D::DrawWireSphere(const glm::vec3 &position,
                                const glm::vec3 &scale, const glm::vec3 &color,
                                float transparancy) {
  UE_PROFILE_FUNCTION();
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  DrawSphere(position, scale, color, transparancy);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer3D::DrawSkeleton(
    const ozz::animation::Skeleton &skeleton,
    const std::vector<ozz::math::Float4x4> &modelMatrices,
    const glm::mat4 &modelTransform) {

  UE_CORE_TRACE("joints: {}", skeleton.num_joints());
  UE_CORE_TRACE("model matrices: {}", modelMatrices.size());
  for (int i = 0; i < skeleton.num_joints(); i++) {
    int parent = skeleton.joint_parents()[i];
    if (parent < 0)
      continue;

    alignas(16) float childM[16];
    alignas(16) float parentM[16];

    ozz::math::StorePtr(modelMatrices[i].cols[0], childM + 0);
    ozz::math::StorePtr(modelMatrices[i].cols[1], childM + 4);
    ozz::math::StorePtr(modelMatrices[i].cols[2], childM + 8);
    ozz::math::StorePtr(modelMatrices[i].cols[3], childM + 12);

    ozz::math::StorePtr(modelMatrices[parent].cols[0], parentM + 0);
    ozz::math::StorePtr(modelMatrices[parent].cols[1], parentM + 4);
    ozz::math::StorePtr(modelMatrices[parent].cols[2], parentM + 8);
    ozz::math::StorePtr(modelMatrices[parent].cols[3], parentM + 12);

    glm::vec3 childPos = glm::vec3(glm::make_mat4(childM)[3]);
    glm::vec3 parentPos = glm::vec3(glm::make_mat4(parentM)[3]);

    childPos = glm::vec3(modelTransform * glm::vec4(childPos, 1));
    parentPos = glm::vec3(modelTransform * glm::vec4(parentPos, 1));

    DrawLine(parentPos, childPos, {1, 0, 0, 1});
    Renderer3D::DrawSphere(childPos, 0.02f, {0, 1, 0});
  }
}

void Renderer3D::DrawLine(const glm::vec3 &p0, const glm::vec3 &p1,
                          const glm::vec4 &color) {
  UE_PROFILE_FUNCTION();
  glm::vec3 points[2] = {p0, p1};

  glBindVertexArray(lineVAO);

  glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);

  m_LineShader->Bind();
  m_LineShader->SetFloat4("u_Color", color);

  glDrawArrays(GL_LINES, 0, 2);
  // glBindVertexArray(0);
}

void Renderer3D::DrawCameraFrustum(const UE::SceneCamera &cam) {
  UE_PROFILE_FUNCTION();
  using namespace UE;

  glm::vec3 pos = cam.GetPosition();
  glm::vec3 forward = cam.GetForwardDirection();
  glm::vec3 right = cam.GetRightDirection();
  glm::vec3 up = cam.GetUpDirection();

  float nearD = cam.GetPerspectiveNearClip();
  float farD = cam.GetPerspectiveFarClip();
  float fov = cam.GetVerticalFOV();
  float ar = cam.GetAspectRatio();

  float nearH = 2.0f * tan(fov / 2.0f) * nearD;
  float nearW = nearH * ar;
  float farH = 2.0f * tan(fov / 2.0f) * farD;
  float farW = farH * ar;

  glm::vec3 nc = pos + forward * nearD;
  glm::vec3 fc = pos + forward * farD;

  glm::vec3 ntl = nc + (up * nearH / 2.0f) - (right * nearW / 2.0f);
  glm::vec3 ntr = nc + (up * nearH / 2.0f) + (right * nearW / 2.0f);
  glm::vec3 nbl = nc - (up * nearH / 2.0f) - (right * nearW / 2.0f);
  glm::vec3 nbr = nc - (up * nearH / 2.0f) + (right * nearW / 2.0f);

  glm::vec3 ftl = fc + (up * farH / 2.0f) - (right * farW / 2.0f);
  glm::vec3 ftr = fc + (up * farH / 2.0f) + (right * farW / 2.0f);
  glm::vec3 fbl = fc - (up * farH / 2.0f) - (right * farW / 2.0f);
  glm::vec3 fbr = fc - (up * farH / 2.0f) + (right * farW / 2.0f);

  // glm::mat4 vp = cam.GetViewProjection();
  glm::vec4 color = {1, 1, 0, 1}; // Yellow

  Renderer3D::DrawLine(ntl, ntr, color);
  Renderer3D::DrawLine(ntr, nbr, color);
  Renderer3D::DrawLine(nbr, nbl, color);
  Renderer3D::DrawLine(nbl, ntl, color);

  Renderer3D::DrawLine(ftl, ftr, color);
  Renderer3D::DrawLine(ftr, fbr, color);
  Renderer3D::DrawLine(fbr, fbl, color);
  Renderer3D::DrawLine(fbl, ftl, color);

  Renderer3D::DrawLine(ntl, ftl, color);
  Renderer3D::DrawLine(ntr, ftr, color);
  Renderer3D::DrawLine(nbr, fbr, color);
  Renderer3D::DrawLine(nbl, fbl, color);
}

} // namespace UE

#type vertex
#version 460 core

// =========================
// Limits
// =========================
#define MAX_BONES 100
#define MAX_BONE_INFLUENCE 4

// =========================
// Vertex Inputs
// =========================
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in ivec4 a_BoneIds;
layout(location = 5) in vec4 a_Weights;

// =========================
// Uniforms
// =========================
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform mat4 u_FinalBonesMatrices[MAX_BONES];

// =========================
// Outputs
// =========================
out VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    vec3 Tangent;
} vs_out;

// =========================
// Main
// =========================
void main()
{
    vec4 skinnedPosition = vec4(0.0);
    vec3 skinnedNormal = vec3(0.0);
    vec3 skinnedTangent = vec3(0.0);

    // ---- Skinning Loop ----
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if(a_BoneIds[i] < 0) continue;
        if(a_BoneIds[i] >= MAX_BONES) break;

        mat4 boneMatrix = u_FinalBonesMatrices[a_BoneIds[i]];
        float weight = a_Weights[i];

        skinnedPosition += boneMatrix * vec4(a_Position, 1.0) * weight;
        skinnedNormal   += mat3(boneMatrix) * a_Normal * weight;
        skinnedTangent  += mat3(boneMatrix) * a_Tangent * weight;
    }

    // ---- Fallback if no valid bones ----
    if (length(skinnedPosition.xyz) < 0.0001)
    {
        skinnedPosition = vec4(a_Position, 1.0);
        skinnedNormal   = a_Normal;
        skinnedTangent  = a_Tangent;
    }

    // ---- World Space ----
    vec4 worldPos = u_Model * skinnedPosition;

    vs_out.FragPos = worldPos.xyz;
    vs_out.TexCoord = a_TexCoord;

    mat3 normalMatrix = transpose(inverse(mat3(u_Model)));

    vs_out.Normal  = normalize(normalMatrix * skinnedNormal);
    vs_out.Tangent = normalize(normalMatrix * skinnedTangent);

    gl_Position = u_Projection * u_View * worldPos;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int  o_EntityID;

// =========================
// Inputs
// =========================
in VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    vec3 Tangent;
} fs_in;

// =========================
// Uniforms
// =========================
uniform int u_EntityID;

uniform vec3 u_CamPos;

// Material
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AOMap;

// Light (simple directional for now)
uniform vec3 u_LightDir;
uniform vec3 u_LightColor;

// =========================
// Constants
// =========================
const float PI = 3.14159265359;

// =========================
// Helpers
// =========================
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(u_NormalMap, fs_in.TexCoord).xyz * 2.0 - 1.0;

    vec3 N = normalize(fs_in.Normal);
    vec3 T = normalize(fs_in.Tangent);
    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// =========================
// Main
// =========================
void main()
{
    vec3 albedo     = pow(texture(u_AlbedoMap, fs_in.TexCoord).rgb, vec3(2.2));
    float metallic  = texture(u_MetallicMap, fs_in.TexCoord).r;
    float roughness = texture(u_RoughnessMap, fs_in.TexCoord).r;
    float ao        = texture(u_AOMap, fs_in.TexCoord).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(u_CamPos - fs_in.FragPos);
    vec3 L = normalize(-u_LightDir);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    // Fresnel
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 F = F0 + (1.0 - F0) * pow(1.0 - max(dot(H, V), 0.0), 5.0);

    // Geometry term (Schlick-GGX)
    float k = (roughness + 1.0);
    k = (k * k) / 8.0;

    float G_V = NdotV / (NdotV * (1.0 - k) + k);
    float G_L = NdotL / (NdotL * (1.0 - k) + k);
    float G = G_V * G_L;

    // Distribution GGX
    float a = roughness * roughness;
    float a2 = a * a;

    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    float D = a2 / (PI * denom * denom);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 diffuse = kD * albedo / PI;

    vec3 radiance = u_LightColor;

    vec3 Lo = (diffuse + specular) * radiance * NdotL;

    // Ambient
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tonemap
    color = color / (color + vec3(1.0));

    // Gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    o_Color = vec4(color, 1.0);
    o_EntityID = u_EntityID;
}
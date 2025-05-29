#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_Normal;
out vec3 v_FragPos;
out vec2 v_TexCoord;

void main() {
    v_FragPos = vec3(u_Model * vec4(a_Position, 1.0));
    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_TexCoord = a_TexCoord;

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}

#type fragment
#version 450 core


in vec3 v_Normal;
in vec3 v_FragPos;
in vec2 v_TexCoord;

out vec4 FragColor;

uniform vec3 u_LightPos;
uniform vec3 u_ViewPos;
uniform sampler2D u_DiffuseTexture;

void main() {
    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * vec3(1.0);

    // Diffuse
    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(u_LightPos - v_FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0);

    vec3 lighting = (ambient + diffuse + specular);

    vec4 texColor = texture(u_DiffuseTexture, v_TexCoord);
    // if (texColor.a == 0.0) // or texColor.rgb == vec3(0)
    //     texColor = vec4(1.0); // fallback to white

    // if(texColor.rgb == vec3(0))
    //     texColor = vec4(1.0);
    FragColor = vec4(lighting, 1.0) * texColor;
    // FragColor = vec4(lighting, 1.0);
    // FragColor = vec4(1.0, 1.0, 0.2, 1.0);
    // FragColor = vec4(v_TexCoord, 0.0, 1.0);
}
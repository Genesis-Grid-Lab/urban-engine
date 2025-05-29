#type vertex
#version 330 core
layout(location = 0) in vec3 a_Position;

out vec3 TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    TexCoords = a_Position;
    vec4 pos = u_Projection * u_View * vec4(a_Position, 1.0);
    gl_Position = pos.xyww; // force depth = 1.0
}

#type fragment
#version 330 core
in vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube u_Skybox;

void main()
{
    FragColor = texture(u_Skybox, TexCoords);
}

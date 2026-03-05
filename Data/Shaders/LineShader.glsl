#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main(){
    gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 FragColor;

uniform vec4 u_Color;

void main(){
    FragColor = u_Color;
}
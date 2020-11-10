#version 450

uniform float u_time;

in vec3 particule_color;

out vec4 color;

void main()
{
    color = vec4(particule_color, 1);
}
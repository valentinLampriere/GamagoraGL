#version 450

uniform float u_time;

in vec3 position;
in vec3 color;
in vec3 velocity;
in float size;

out vec3 particule_color;

void main()
{
    particule_color = color;
    gl_Position = vec4(position + velocity, 1.0);
    gl_PointSize = size * (sin(u_time) + 1.f) / 2.f;
}
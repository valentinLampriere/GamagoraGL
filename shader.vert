#version 450

uniform float u_time;
uniform vec2 mousePos;

in vec3 position;
in float size;

void main() {
    gl_Position = vec4(position * 5. - vec3(0,0.5,0), 1.0);
}
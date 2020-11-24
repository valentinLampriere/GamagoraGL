#version 450

uniform mat4 view;
uniform mat4 proj;
uniform float u_time;
uniform sampler2D tex;
uniform vec3 lightPosition;

in vec3 position;
in vec2 textCoord;
in vec3 normal_2;

out vec2 UV;
out vec3 normal;
out vec3 pos;

void main() {
    UV = textCoord;
    normal = normal_2;
    pos = position;


    gl_Position = proj * view * vec4(position, 1.0);
}
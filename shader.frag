#version 450

uniform float u_time;
uniform sampler2D tex;

out vec4 color;

in vec2 UV;
in vec4 v_color;

void main() {
    color = vec4(texture(tex, UV).rgb * v_color.rgb, 1);
}
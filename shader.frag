#version 450

uniform float u_time;
uniform sampler2D tex;

out vec4 color;

in vec3 normal;
in vec2 UV;
in vec3 pos;

void main() {

    vec3 lightPosition = vec3(100, 100, 500);
    vec3 lightDirection = normalize(lightPosition - pos);
    float coeff = abs(dot(lightDirection, normal));
    vec4 v_color = vec4(coeff * vec3(0.9, 0.9, 0.8), 1);

    color = vec4(texture(tex, UV).rgb * v_color.rgb, 1);
}
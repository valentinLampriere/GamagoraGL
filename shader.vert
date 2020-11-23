#version 450

uniform mat4 view;
uniform mat4 proj;
uniform float u_time;
uniform sampler2D tex;

in vec3 position;
in vec2 textCoord;
in vec3 normal;

out vec2 UV;
out vec4 v_color;

void main() {
    UV = textCoord;

    vec3 lightPosition = vec3(100, 100, 500);
    vec3 lightDirection = normalize(lightPosition - gl_Position.xyz);
    //float coeff = abs(dot(lightDirection, normal));
    float coeff = 1.;
    v_color = vec4(coeff * vec3(0.9, 0.9, 0.8), 1);
    
    gl_Position = proj * view * vec4(position, 1.0);
}
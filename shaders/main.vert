#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 texCoords;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform PushConstant {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(aPos, 1.0);
    fragColor = aPos.xyz;
}

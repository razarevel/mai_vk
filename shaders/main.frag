#version 460

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 color;
layout(location = 1) out vec2 fragTexCoord;

layout(location = 0) out vec4 out_FragColor;

void main() {
    out_FragColor = texture(texSampler, fragTexCoord);
}

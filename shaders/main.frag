#version 460 core

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 texCoords;

layout(location = 0) out vec4 out_FragColor;

layout(binding = 1) uniform sampler2D image;

void main() {
    out_FragColor = texture(image, texCoords);
}

#version 460 core

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 texCoords;

layout(location = 0) out vec4 out_FragColor;

void main() {
    out_FragColor = vec4(texCoords, 0.0f, 1.0f);
}

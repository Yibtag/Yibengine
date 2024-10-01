#version 450

layout (push_constant) uniform PushContant {
    mat4 transform;
} push_constant;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec3 fragColor;

void main() {
    fragColor = color;

    gl_Position = push_constant.transform * vec4(
        position,
        1.0
    );
}
#version 450

layout (push_constant) uniform PushContant {
    mat4 model_matrix;
} push_constant;


layout (location = 0) in vec3 fragColor;
layout (location = 0) out vec4 color;

void main() {
    color = vec4(fragColor, 1.0);
}
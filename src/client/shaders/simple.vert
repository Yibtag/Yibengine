#version 450

layout (push_constant) uniform PushContant {
    mat4 model_matrix;
} push_constant;

layout (set = 0, binding = 0) uniform UBO {
    mat4 projection_view_matrix;
} ubo;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec2 fragUV;

void main() {
    fragUV =  uv;

    gl_Position = ubo.projection_view_matrix * push_constant.model_matrix * vec4(
        position,
        1.0
    );
}
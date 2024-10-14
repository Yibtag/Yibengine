#version 450

layout (set = 0, binding = 1) uniform sampler2D image;

layout (location = 0) in vec2 fragUV;

layout (location = 0) out vec4 color;

void main() {
    color = vec4(texture(image, fragUV).rgb, 1.0);
}
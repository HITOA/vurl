#version 450

#define M_PI 3.1415926535897932384626433832795
#define M_INV_PI (1/M_PI)

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inColor;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput inNormal;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(subpassLoad(inColor).rgb, 1.0);
}
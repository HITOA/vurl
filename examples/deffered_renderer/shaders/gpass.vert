#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 outNormal;

layout(push_constant) uniform MeshPushConstant {
    mat4 mvp;
    mat4 projection;
    mat4 view;
    mat4 model;
} constant;

void main() {
    gl_Position = constant.mvp * vec4(position, 1.0);
    outNormal = normal;
}
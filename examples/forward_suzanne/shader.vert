#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 outNormal;

layout(push_constant) uniform TransformationData {
    mat4 mvp;
} transformationData;

void main() {
    gl_Position = transformationData.mvp * vec4(position, 1.0);
    outNormal = normal;
}
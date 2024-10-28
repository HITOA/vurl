#version 450

#define M_PI 3.1415926535897932384626433832795
#define M_INV_PI (1/M_PI)

layout(location = 0) in vec3 normal;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outNormal;

vec2 encode(vec3 n) {
    vec2 f;
    f.x = atan(n.y, n.x) * M_INV_PI;
    f.y = n.z;

    return f * 0.5 + 0.5;
}

void main() {
    outColor = vec3(1.0, 1.0, 1.0);
    outNormal = encode(normal);
}
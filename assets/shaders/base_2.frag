#version 450

// layout(location = 0) in vec3 fVertex;
// layout(location = 1) in vec2 fUv;
// layout(location = 2) in vec3 fNormal;
// layout(location = 3) in vec3 fTangent;
// layout(location = 4) in vec3 fBitanget;
layout(location = 5) in vec3 fColor;

layout(location = 0) out vec4 outColor;

void main() { outColor = vec4(fColor, 1.0); }

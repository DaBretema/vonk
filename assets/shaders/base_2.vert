#version 450

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitanget;
layout(location = 5) in vec3 color;

layout(location = 0) out vec3 fVertex;
layout(location = 1) out vec2 fUv;
layout(location = 2) out vec3 fNormal;
layout(location = 3) out vec3 fTangent;
layout(location = 4) out vec3 fBitanget;
layout(location = 5) out vec3 fColor;

void main()
{
  gl_Position = vec4(vertex, 1.0);

  fVertex   = vertex;
  fUv       = uv;
  fNormal   = normal;
  fTangent  = tangent;
  fBitanget = bitanget;
  fColor    = color;
}

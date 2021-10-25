#version 450

layout(location = 0) out vec3 fragColor;

#define SHAPE 2
#define COLOR 1

// . Colors
const vec3 W     = vec3(1.);
const vec3 BASE  = vec3(1., .5, 0.);
const vec3 BASE2 = vec3(1., .2, 0.);
#if 0
const vec3 R = BASE * .5;
const vec3 G = BASE2 * .1;
const vec3 B = BASE2 * .5;
#else
const vec3 R            = vec3(1., 0., 0.);
const vec3 G            = vec3(0., 1., 0.);
const vec3 B            = vec3(0., 0., 1.);
#endif

// . Points
const float v  = .9;
const vec2  CU = vec2(.0, -v);
const vec2  CB = vec2(.0, v);
const vec2  RB = vec2(v, v);
const vec2  LB = vec2(-v, v);
const vec2  RU = vec2(v, -v);
const vec2  LU = vec2(-v, -v);

const vec2 hy = vec2(.0, v * .5);
const vec2 qy = vec2(.0, hy.y * .5);
const vec2 hx = vec2(v * .5, 0.);
const vec2 qx = vec2(hx.x * .5, 0.);
const vec2 ox = vec2(qx.x * .5, 0.);

// . Shape
#if SHAPE == 0  // .. Triangle
vec2 positions[3] = vec2[](CU, RB, LB);
#elif SHAPE == 1  // .. Quad
vec2       positions[6] = vec2[](LU, RU, RB, RB, LB, LU);
#elif SHAPE == 2  // .. Arrow
vec2 positions[6] = vec2[](CU, RB - ox, CB - hy, CB - hy, LB + ox, CU);
#elif SHAPE == 3  // .. Diamond
vec2 positions[6] = vec2[](CU, RB - (ox + hy) * 2, CB, CB, LB - (-ox + hy) * 2, CU);
#endif

// . Color
#if SHAPE == 0
#  if COLOR == 0
vec3 colors[3] = vec3[](W, W, W);
#  else
vec3 colors[3] = vec3[](R, G, B);
#  endif
#else
#  if COLOR == 0
vec3       colors[6]    = vec3[](W, W, W, W, W, W);
#  else
vec3 colors[6] = vec3[](R, G, B, B, G, R);
#  endif
#endif

// . EntryPoint
void main()
{
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
  fragColor   = colors[gl_VertexIndex];
}

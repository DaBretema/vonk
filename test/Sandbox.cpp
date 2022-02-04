// #include "VonkWindow.h"
// #include "Vonk.h"

#include "_glm.h"
#include "Macros.h"

#include <vector>

namespace ddd
{  //

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// CPP TOOLING
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
std::string floatStr(float f, uint32_t decs = 2)
{
  std::string s     = "";
  bool        isDec = false;
  uint32_t    count = 0;
  for (auto const &ch : std::to_string(f)) {
    if (isDec) {
      if (count >= decs) break;
      ++count;
    }
    if (ch == '.') { isDec = true; }
    s += ch;
  }
  return s;
};
std::string vecStr(auto const &v, uint32_t decs = 2)
{
  std::string s = "{ ";
  for (size_t i = 0; i < v.length() - 1; ++i) { s += " " + floatStr(v[i], decs) + ","; }
  s += " " + floatStr(v[v.length() - 1], decs) + " }";
  return s;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// . CONST : Factors / Steps
MBU float constexpr toRadians { 3.14159f / 180.f };
MBU float constexpr toDegrees { 180.f / 3.14159f };

// . CONST : Vecs
MBU float constexpr One { 1.f };
MBU glm::vec2 constexpr One2 { One, One };
MBU glm::vec3 constexpr One3 { One, One, One };
MBU glm::vec4 constexpr One4 { One, One, One, One };

MBU float constexpr Zero { 0.f };
MBU glm::vec2 constexpr Zero2 { Zero, Zero };
MBU glm::vec3 constexpr Zero3 { Zero, Zero, Zero };
MBU glm::vec4 constexpr Zero4 { Zero, Zero, Zero, Zero };
MBU glm::vec4 constexpr Zero4x { Zero, Zero, Zero, One };

MBU glm::vec3 constexpr AxisX { One, Zero, Zero };
MBU glm::vec3 constexpr AxisY { Zero, One, Zero };
MBU glm::vec3 constexpr AxisZ { Zero, Zero, One };

// . CONST : Colors Primary
MBU glm::vec4 constexpr White { One4 };
MBU glm::vec4 constexpr Black { Zero4x };
MBU glm::vec4 constexpr Red { One, Zero, Zero, One };
MBU glm::vec4 constexpr Green { Zero, One, Zero, One };
MBU glm::vec4 constexpr Blue { Zero, Zero, One, One };
MBU glm::vec4 constexpr Cyan { Zero, One, One, One };
MBU glm::vec4 constexpr Magenta { One, Zero, One, One };
MBU glm::vec4 constexpr Yellow { One, One, Zero, One };

// . CONST : Colors Mix
MBU glm::vec4 constexpr Gray01 { 0.1f, 0.1f, 0.1f, One };
MBU glm::vec4 constexpr Gray02 { 0.2f, 0.2f, 0.2f, One };
MBU glm::vec4 constexpr Gray03 { 0.3f, 0.3f, 0.3f, One };
MBU glm::vec4 constexpr Gray04 { 0.4f, 0.4f, 0.4f, One };
MBU glm::vec4 constexpr Gray05 { 0.5f, 0.5f, 0.5f, One };
MBU glm::vec4 constexpr Gray06 { 0.6f, 0.6f, 0.6f, One };
MBU glm::vec4 constexpr Gray07 { 0.7f, 0.7f, 0.7f, One };
MBU glm::vec4 constexpr Gray08 { 0.8f, 0.8f, 0.8f, One };
MBU glm::vec4 constexpr Gray09 { 0.9f, 0.9f, 0.9f, One };

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// GLM TOOLING
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
glm::mat4 rotateXYZ(glm::mat4 &mat, glm::vec3 const &angles)
{
  mat = glm::rotate(mat, glm::radians(angles.z), AxisZ);
  mat = glm::rotate(mat, glm::radians(angles.y), AxisY);
  mat = glm::rotate(mat, glm::radians(angles.x), AxisX);
  return mat;
}

glm::mat4 rotVecToMat4(glm::vec3 const &angles)
{
  auto T = glm::mat4(1.f);
  return rotateXYZ(T, angles);
}

auto rotFromTo(glm::vec3 const &A, glm::vec3 const &B)
{
  auto const nA = glm::normalize(A);
  auto const nB = glm::normalize(B);
  auto const q  = glm::rotation(nA, nB);
  return glm::eulerAngles(q) * ddd::toDegrees;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Mesh
{
  struct Vertex
  {
    glm::vec3 pos { Zero3 };
    glm::vec2 uv { Zero2 };
    glm::vec3 normal { Zero3 };
    glm::vec3 tangent { Zero3 };
    glm::vec3 bitangent { Zero3 };
    glm::vec4 color { Zero4x };
  };

  struct Instance
  {
    glm::mat4 transform { 1.f };
    glm::vec4 color { Magenta };
  };

  std::vector<uint32_t> indices {};
  std::vector<Vertex>   vertices {};
  std::vector<Instance> instances {};
};

//----

struct Camera
{
  enum Mode : uint32_t
  {
    Orbital,  // 1. rot 2. pos
    Fly       // 1. pos 2. rot
  };
  enum Type : uint32_t
  {
    Perspective,
    Orthogonal
  };
  glm::vec3 eye { AxisZ * -10.f };       // Points  // or... glm::vec3 pos { AxisZ * -10.f }; (Point)
  glm::vec3 lookat { Zero3 };            // Points  // or... glm::vec3 rot { Zero3 }; (Euler Angles)
  float     fov { glm::radians(45.f) };  // [0.5 to 170] Degrees (From Blender)
  Mode      mode { Mode::Orbital };
  Type      type { Type::Perspective };
  // . And use pos*rot to Fly and rot*pos to Orbit
};

//----

struct Light  // To direct use on UBOs : Keep it aligned
{
  // On Shader: struct Light { vec4 pos; vec4 color; vec4 dir; float type; float I; float angle; float blend; };

  enum Type : uint32_t
  {
    Point,
    Spot,
    Dir
  };

  glm::vec4 pos { AxisY * 5.f, 1.f };  // .w==0 means disabled
  glm::vec4 color { Yellow };
  glm::vec4 dir { Zero4 };  // (Eurler Angles) Apply to (0,-1,0)

  float type     = (float)Point;
  float intesity = 1.f;   // [ 0.0 : 100'000 ] on Watts
  float angle    = 0.2f;  // [ 0.0 : PI ]
  float blend    = 0.2f;  // [ 0.0 : 1.0 ]
};

glm::vec3 getLightFront(Light const &light) { return glm::normalize(rotVecToMat4(light.dir)[2]); }

//----

struct Texture
{
  //...
};

//----

struct Material
{
  struct
  {
    glm::vec4 albedo { Magenta };
    glm::vec4 emissive { Black };
  } color;

  struct
  {
    float metallic     = 0.f;  // [ 0.0 - 1.0 ]
    float roughness    = 0.f;  // [ 0.0 - 1.0 ]
    float transmission = 0.f;  // [ 0.0 - 1.0 ]
  } factor;

  struct
  {
    std::string albedo       = "";
    std::string emissive     = "";
    std::string metallic     = "";
    std::string roughness    = "";
    std::string transmission = "";
  } texpath;
};

//----

struct Scene
{
  // Hierarchy ...
  std::vector<Material> materials;
  std::vector<Camera>   cameras;
  std::vector<Light>    lights;
  std::vector<Mesh>     meshes;
};

//----

}  // namespace ddd

#define TEST_N 0

int main()
{
  // TASKS
  // [x] Define base structs
  // [ ] Base structus CRUD methods ¿?
  // [ ] Load a scene with assimp
  // [ ] Write Hierarchy logic
  // [ ] ddd::gpu <- meshes, fbos, textures, images, buffers, postpro, batching, sort by material, sort by opacity.

  // === Quick Maths

#if TEST_N == 1
  glm::vec3 const P0 { ddd::AxisY * 6.f };
  glm::vec3 const P1 { ddd::Zero3 };
  glm::vec3 const P2 { (ddd::AxisX + ddd::AxisZ) * 100.f };

  glm::vec3 const V1 { P1 - P0 };
  glm::vec3 const V2 { P2 - P0 };

  LogInfof("::: {}", ddd::vecStr(V1));
  LogInfof("::: {}", ddd::vecStr(V2));
  LogInfof("::: {}", ddd::vecStr(ddd::rotFromTo(V1, V2)));
#else
#endif
}

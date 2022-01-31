// #include "VonkWindow.h"
// #include "Vonk.h"

#include "_glm.h"
#include "Macros.h"

#include <vector>

namespace ddd::cpu
{  //

// . CONST : Vecs
float constexpr One { 1.f };
glm::vec2 constexpr One2 { One, One };
glm::vec3 constexpr One3 { One, One, One };
glm::vec4 constexpr One4 { One, One, One, One };

float constexpr Zero { 0.f };
glm::vec2 constexpr Zero2 { Zero, Zero };
glm::vec3 constexpr Zero3 { Zero, Zero, Zero };
glm::vec4 constexpr Zero4 { Zero, Zero, Zero, Zero };
glm::vec4 constexpr Zero4x { Zero, Zero, Zero, One };

glm::vec3 constexpr AxisX { One, Zero, Zero };
glm::vec3 constexpr AxisY { Zero, One, Zero };
glm::vec3 constexpr AxisZ { Zero, Zero, One };

// . CONST : Colors Primary
glm::vec4 constexpr White { One4 };
glm::vec4 constexpr Black { Zero4x };
glm::vec4 constexpr Red { One, Zero, Zero, One };
glm::vec4 constexpr Green { Zero, One, Zero, One };
glm::vec4 constexpr Blue { Zero, Zero, One, One };
glm::vec4 constexpr Cyan { Zero, One, One, One };
glm::vec4 constexpr Magenta { One, Zero, One, One };
glm::vec4 constexpr Yellow { One, One, Zero, One };

// . CONST : Colors Mix
glm::vec4 constexpr Gray01 { 0.1f, 0.1f, 0.1f, One };
glm::vec4 constexpr Gray02 { 0.2f, 0.2f, 0.2f, One };
glm::vec4 constexpr Gray03 { 0.3f, 0.3f, 0.3f, One };
glm::vec4 constexpr Gray04 { 0.4f, 0.4f, 0.4f, One };
glm::vec4 constexpr Gray05 { 0.5f, 0.5f, 0.5f, One };
glm::vec4 constexpr Gray06 { 0.6f, 0.6f, 0.6f, One };
glm::vec4 constexpr Gray07 { 0.7f, 0.7f, 0.7f, One };
glm::vec4 constexpr Gray08 { 0.8f, 0.8f, 0.8f, One };
glm::vec4 constexpr Gray09 { 0.9f, 0.9f, 0.9f, One };

//----

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
  // glm::vec3 eye { AxisZ * -10.f };
  // glm::vec3 lookat { Zero3 };
  // or
  glm::vec3 pos { AxisZ * -10.f };
  glm::vec3 rot { Zero3 };
  float     fov = 1.7f;
  // . And use pos*rot to Fly and rot*pos to Orbit
};

//----

struct Light  // To direct use on UBOs : Keep it aligned
{
  // On Shader: struct Light { vec4 pos; vec4 color; vec4 dir; float type; float I; float angle; float blend; };

  enum Type : int32_t
  {
    Point,
    Spot,
    Dir
  };

  glm::vec4 pos { AxisY * 5.f, 1.f };  // .w==0 means disabled
  glm::vec4 color { Yellow };
  glm::vec4 dir { Zero4 };  // The rot to apply to (0,-1,0)

  float type     = (float)Point;
  float intesity = 1.f;   // [ 0 : 100'000 ] on Watts
  float angle    = 0.2f;  // [ 0 : PI ]
  float blend    = 0.2f;  // [ 0 : 1 ]
};

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
    float metallic     = 0.f;
    float roughness    = 0.f;
    float transmission = 0.f;
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

}  // namespace ddd::cpu

int main()
{
  // TASKS
  // [x] Define base structs
  // [ ] Base structus CRUD methods ¿?
  // [ ] Load a scene with assimp
  // [ ] Write Hierarchy logic
  // [ ] ddd::gpu <- meshes, fbos, textures, images, buffers, postpro, batching, sort by material, sort by opacity.

  glm::vec3 a(1, 2, 3);
  LogInfof("{}", glm::to_string(ddd::cpu::Gray02));
  LogInfof("{}", glm::to_string(ddd::cpu::Gray04));
  LogInfof("{}", glm::to_string(ddd::cpu::Gray06));
  LogInfof("{}", glm::to_string(ddd::cpu::Gray08));
}

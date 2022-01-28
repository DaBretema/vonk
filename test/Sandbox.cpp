// #include "VonkWindow.h"
// #include "Vonk.h"

#include "_glm.h"
#include "Macros.h"

#include <vector>

namespace ddd::data
{  //

struct Mesh
{
  struct Vertex
  {
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec4 tangent;
    glm::vec4 bitangent;
    glm::vec4 color;
  };

  struct Instance
  {
    glm::mat4 transform;
    glm::vec4 color;
  };

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  std::vector<Instance> instances;
};

struct Camera
{
  glm::vec3 eye;
  glm::vec3 lookat;
  // or
  glm::vec3 pos;
  glm::vec3 rot;
  // . And use pos*rot to Fly and rot*pos to Orbit
};

struct Light  // To direct use on UBOs : Keep it aligned
{
  // On Shader: struct Light { vec4 pos; vec4 color; vec4 dir; float type; float I; float angle; float blend; };

  enum Type : int32_t
  {
    Point,
    Spot,
    Dir
  };

  glm::vec4 pos;  // .w==0 means disabled
  glm::vec4 color;
  glm::vec4 dir;  // The rot to apply to (0,-1,0)

  float type     = (float)Point;
  float intesity = 1.f;   // [ 0 : 100'000 ] on Watts
  float angle    = 0.2f;  // [ 0 : PI ]
  float blend    = 0.2f;  // [ 0 : 1 ]
};

struct Texture
{
  //...
};

struct Material
{
  struct
  {
    glm::vec4 albedo;
    glm::vec4 emissive;
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

struct Scene
{
  // Hierarchy ...
  std::vector<Material> materials;
  std::vector<Camera>   cameras;
  std::vector<Light>    lights;
  std::vector<Mesh>     meshes;
};

}  // namespace ddd::data

int main()
{
  glm::vec3 a(1, 2, 3);
  LogInfof("{}", glm::to_string(a));
}

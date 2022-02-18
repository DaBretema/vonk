// #include "VonkWindow.h"
// #include "Vonk.h"

#include "Macros.h"
#include "_glm.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <sole.hpp>

#include <string>

namespace dc
{ //

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// CPP TOOLING
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// class HierarchyTree
// {
// public:
//   struct Node
//   {
//     enum class Type
//     {
//       Node,
//       Mesh,
//       Light,
//       Camera
//     };  // Add extra data-structs to allow O(1) search/iteration by Type.

//     Type type = Type::Node;

//     std::string name  = "";
//     uuid_t      uuid  = "";
//     uint32_t    level = 0;
//     glm::mat4   transform { 1.f };

//     // std::vector<uint32_t> meshesIdx {};  // Avoid this injecting everything as Node and filtering by Type

//     uuid_t              parent = "";
//     std::vector<uuid_t> childs = {};
//   };

//   inline uuid_t addNode(Node::Type type, std::string const &name, glm::mat4 const &T, uuid_t const &parent)
//   {
//     Node *p = (mHierarchy.count(parent) > 0) ? &mHierarchy.at(parent) : nullptr;

//     uuid_t const   uuid  = sole::uuid4().str();
//     uint32_t const level = p ? p->level + 1 : 0;

//     Node const node {
//       .type      = type,
//       .name      = name,
//       .uuid      = uuid,
//       .level     = level,
//       .transform = T,
//       .parent    = parent,
//     };

//     if (p) { p->childs.push_back(uuid); }

//     mHierarchy.insert_or_assign(uuid, node);
//   }

// private:
//   std::unordered_map<uuid_t, Node> mHierarchy = {};
// };
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline std::string toStr(float f, uint32_t d = 2)
{
    return fmt::format("{:.{}f}", f, d);
};
inline std::string toStr(glm::vec3 const &v, uint32_t d = 2)
{
    return fmt::format("({},{},{})", toStr(v.x, d), toStr(v.y, d), toStr(v.z, d));
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// . CONST : Factors / Steps
MBU float constexpr toRadians{3.14159f / 180.f};
MBU float constexpr toDegrees{180.f / 3.14159f};

// . CONST : Vecs
MBU float constexpr One{1.f};
MBU glm::vec2 constexpr One2{One, One};
MBU glm::vec3 constexpr One3{One, One, One};
MBU glm::vec4 constexpr One4{One, One, One, One};

MBU float constexpr Zero{0.f};
MBU       glm::vec2 constexpr Zero2{Zero, Zero};
MBU       glm::vec3 constexpr Zero3{Zero, Zero, Zero};
MBU       glm::vec4 constexpr Zero4{Zero, Zero, Zero, Zero};
MBU       glm::vec4 constexpr Zero4x{Zero, Zero, Zero, One};

MBU       glm::vec3 constexpr AxisX{One, Zero, Zero};
MBU       glm::vec3 constexpr AxisY{Zero, One, Zero};
MBU       glm::vec3 constexpr AxisZ{Zero, Zero, One};

// . CONST : Colors Primary
MBU       glm::vec4 constexpr White{One4};
MBU       glm::vec4 constexpr Black{Zero4x};
MBU       glm::vec4 constexpr Red{One, Zero, Zero, One};
MBU       glm::vec4 constexpr Green{Zero, One, Zero, One};
MBU       glm::vec4 constexpr Blue{Zero, Zero, One, One};
MBU       glm::vec4 constexpr Cyan{Zero, One, One, One};
MBU       glm::vec4 constexpr Magenta{One, Zero, One, One};
MBU       glm::vec4 constexpr Yellow{One, One, Zero, One};

// . CONST : Colors Mix
MBU       glm::vec4 constexpr Gray01 = {0.1f, 0.1f, 0.1f, One};
MBU       glm::vec4 constexpr Gray02 = {0.2f, 0.2f, 0.2f, One};
MBU       glm::vec4 constexpr Gray03 = {0.3f, 0.3f, 0.3f, One};
MBU       glm::vec4 constexpr Gray04 = {0.4f, 0.4f, 0.4f, One};
MBU       glm::vec4 constexpr Gray05 = {0.5f, 0.5f, 0.5f, One};
MBU       glm::vec4 constexpr Gray06 = {0.6f, 0.6f, 0.6f, One};
MBU       glm::vec4 constexpr Gray07 = {0.7f, 0.7f, 0.7f, One};
MBU       glm::vec4 constexpr Gray08 = {0.8f, 0.8f, 0.8f, One};
MBU       glm::vec4 constexpr Gray09 = {0.9f, 0.9f, 0.9f, One};

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
    return glm::eulerAngles(q) * dc::toDegrees;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Mesh
{
    struct Vertex
    {
        glm::vec3 pos       = Zero3;
        glm::vec2 uv        = Zero2;
        glm::vec3 normal    = Zero3;
        glm::vec3 tangent   = Zero3;
        glm::vec3 bitangent = Zero3;
        glm::vec4 color     = Zero4x;
    };

    struct Instance
    {
        glm::mat4 transform{1.f};
        glm::vec4 color = Magenta;
    };

    std::vector<uint32_t> indices   = {};
    std::vector<Vertex>   vertices  = {};
    std::vector<Instance> instances = {};
};

//----

struct Camera
{
    enum Mode : uint32_t
    {
        Orbital, // 1. rot 2. pos
        Fly      // 1. pos 2. rot
    };
    enum Type : uint32_t
    {
        Perspective,
        Orthogonal
    };
    glm::vec3 eye    = AxisZ * -10.f;      // Points  // or... glm::vec3 pos { AxisZ * -10.f }; (Point)
    glm::vec3 lookat = Zero3;              // Points  // or... glm::vec3 rot { Zero3 }; (Euler Angles)
    float     fov    = glm::radians(45.f); // [0.5 to 170] Degrees (From Blender)
    Mode      mode   = Mode::Orbital;
    Type      type   = Type::Perspective;
    // . And use pos*rot to Fly and rot*pos to Orbit
};

//----

struct Light // To direct use on UBOs : Keep it aligned
{
    // On Shader: struct Light { vec4 pos; vec4 color; vec4 dir; float type; float I; float angle; float blend; };

    enum Type : uint32_t
    {
        Point,
        Spot,
        Dir
    };

    glm::vec4 pos      = {AxisY * 5.f, 1.f}; // .w==0 means disabled
    glm::vec4 color    = Yellow;
    glm::vec4 dir      = Zero4; // (Eurler Angles) Apply to (0,-1,0)

    float     type     = (float)Point;
    float     intesity = 1.f;  // [ 0.0 : 100'000 ] on Watts
    float     angle    = 0.2f; // [ 0.0 : PI ]
    float     blend    = 0.2f; // [ 0.0 : 1.0 ]
};

glm::vec3 getLightFront(Light const &light)
{
    return glm::normalize(rotVecToMat4(light.dir)[2]);
}

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
        glm::vec4 albedo   = Magenta;
        glm::vec4 emissive = Black;
    } color;

    struct
    {
        float metallic     = 0.f; // [ 0.0 - 1.0 ]
        float roughness    = 0.f; // [ 0.0 - 1.0 ]
        float transmission = 0.f; // [ 0.0 - 1.0 ]
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
    std::vector<Material> materials = {};
    std::vector<Camera>   cameras   = {};
    std::vector<Light>    lights    = {};
    std::vector<Mesh>     meshes    = {};
};

//----

} // namespace dc

#define TEST_N 1

int main()
{
    // TASKS
    // [x] Define base structs
    // [ ] Base structus CRUD methods ¿?
    // [ ] Load a scene with assimp
    // [ ] Write Hierarchy logic
    // [ ] dc::gpu <- meshes, fbos, textures, images, buffers, postpro, batching, sort by material, sort by opacity.

    // === Quick Maths

#if TEST_N == 1
    glm::vec3 const P0{dc::AxisY * 6.f};
    glm::vec3 const P1{dc::Zero3};
    glm::vec3 const P2{(dc::AxisX + dc::AxisZ) * 100.f};

    glm::vec3 const V1{P1 - P0};
    glm::vec3 const V2{P2 - P0};

    LogInfof("::: {}", dc::toStr(V1));
    LogInfof("::: {}", dc::toStr(V2));
    LogInfof("::: {}", dc::toStr(dc::rotFromTo(V1, V2)));
#else
#endif
}

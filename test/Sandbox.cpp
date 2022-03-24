#include "Vonk.h"
#include "VonkWindow.h"

#include "Macros.h"
#include "_glm.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <entt.hpp> // ECS things

#include <sole.hpp> // UUID thing

//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
// ::: USEFUL LINKS
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*

ECS on C (or as_plain_as_possible):
https://gamedev.stackexchange.com/questions/172584/how-could-i-implement-an-ecs-in-c

*/
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//

namespace dc
{ //

//=====================================
// RENDER CPU SIDE - CONSTANTS
//=====================================

// . Factors / Steps
MBU float constexpr toRadians = 3.14159f / 180.f;
MBU float constexpr toDegrees = 180.f / 3.14159f;

// . Vecs
MBU float constexpr One      = 1.f;
MBU glm::vec2 constexpr One2 = {One, One};
MBU glm::vec3 constexpr One3 = {One, One, One};
MBU glm::vec4 constexpr One4 = {One, One, One, One};

MBU float constexpr Zero       = 0.f;
MBU glm::vec2 constexpr Zero2  = {Zero, Zero};
MBU glm::vec3 constexpr Zero3  = {Zero, Zero, Zero};
MBU glm::vec4 constexpr Zero4  = {Zero, Zero, Zero, Zero};
MBU glm::vec4 constexpr Zero4x = {Zero, Zero, Zero, One};

MBU glm::vec3 constexpr AxisX = {One, Zero, Zero};
MBU glm::vec3 constexpr AxisY = {Zero, One, Zero};
MBU glm::vec3 constexpr AxisZ = {Zero, Zero, One};

// . Colors Primary
MBU glm::vec4 constexpr White   = {One4};
MBU glm::vec4 constexpr Black   = {Zero4x};
MBU glm::vec4 constexpr Red     = {One, Zero, Zero, One};
MBU glm::vec4 constexpr Green   = {Zero, One, Zero, One};
MBU glm::vec4 constexpr Blue    = {Zero, Zero, One, One};
MBU glm::vec4 constexpr Cyan    = {Zero, One, One, One};
MBU glm::vec4 constexpr Magenta = {One, Zero, One, One};
MBU glm::vec4 constexpr Yellow  = {One, One, Zero, One};

// . Colors Mix
MBU glm::vec4 constexpr Gray01 = {0.1f, 0.1f, 0.1f, One};
MBU glm::vec4 constexpr Gray02 = {0.2f, 0.2f, 0.2f, One};
MBU glm::vec4 constexpr Gray03 = {0.3f, 0.3f, 0.3f, One};
MBU glm::vec4 constexpr Gray04 = {0.4f, 0.4f, 0.4f, One};
MBU glm::vec4 constexpr Gray05 = {0.5f, 0.5f, 0.5f, One};
MBU glm::vec4 constexpr Gray06 = {0.6f, 0.6f, 0.6f, One};
MBU glm::vec4 constexpr Gray07 = {0.7f, 0.7f, 0.7f, One};
MBU glm::vec4 constexpr Gray08 = {0.8f, 0.8f, 0.8f, One};
MBU glm::vec4 constexpr Gray09 = {0.9f, 0.9f, 0.9f, One};

//---------------------------------------------------------------------------------------------------------------------
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
//---------------------------------------------------------------------------------------------------------------------

//=====================================
// CPP - TOOLING
//=====================================

// ::: STRINGS

//---

inline std::string toStr(float f, uint32_t d = 2)
{
    return fmt::format("{:.{}f}", f, d);
};

//---

//---------------------------------------------------------------------------------------------------------------------
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
//---------------------------------------------------------------------------------------------------------------------

//=====================================
// GLM - TOOLING
//=====================================

// ::: STRINGS

//---

inline std::string toStr(glm::vec3 const &v, uint32_t d = 2)
{
    return fmt::format("({},{},{})", toStr(v.x, d), toStr(v.y, d), toStr(v.z, d));
};

//---

// ::: ROTATIONS

//---

glm::mat4 rotateXYZ(glm::mat4 &mat, glm::vec3 const &angles)
{
    mat = glm::rotate(mat, glm::radians(angles.z), AxisZ);
    mat = glm::rotate(mat, glm::radians(angles.y), AxisY);
    mat = glm::rotate(mat, glm::radians(angles.x), AxisX);
    return mat;
}

//---

glm::mat4 rotVecToMat4(glm::vec3 const &angles)
{
    auto T = glm::mat4(1.f);
    return rotateXYZ(T, angles);
}

//---

auto rotFromTo(glm::vec3 const &A, glm::vec3 const &B)
{
    auto const nA = glm::normalize(A);
    auto const nB = glm::normalize(B);
    auto const q  = glm::rotation(nA, nB);
    return glm::eulerAngles(q) * dc::toDegrees;
};

//---

//---------------------------------------------------------------------------------------------------------------------
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
//---------------------------------------------------------------------------------------------------------------------

//=====================================
// RENDER CPU SIDE - SCENE
//=====================================

// ::: SCENE RELATED

//---

enum SceneNodeType
{
    Node_tc   = 0,
    Mesh_tc   = 1,
    Light_tc  = 2,
    Camera_tc = 3
};

//---

// @idea : Meshes, Lights and Cameras to INHERIT from this.
struct SceneNode
{
    std::string name      = "";
    std::string uuid      = "";
    uint32_t    level     = 0;
    glm::mat4   transform = glm::mat4(1.f);

    std::string              parentUUID = "";
    std::vector<std::string> childsUUID = {};

    virtual SceneNodeType getType() = 0; // Override this on : mesh, light and camera
};

//---

struct Mesh;
struct Light;
struct Camera;

struct Scene
{
  public:
    inline void addNode(SceneNode *node, std::string const &name, glm::mat4 const &T, std::string const &parentUUID)
    {

        node->name      = name;
        node->transform = T;
        node->uuid      = sole::uuid4().str();

        if (SceneNode *p = (mTree.count(parentUUID) > 0) ? mTree.at(parentUUID) : nullptr; p)
        {
            node->level      = p->level + 1;
            node->parentUUID = parentUUID;
        }

        mTree.insert_or_assign(node->uuid, node);
        // return node->uuid;
    }

    inline SceneNode *getNode(std::string const &uuid)
    {
        return mTree.count(uuid) > 0 ? mTree.at(uuid) : nullptr;
    }
    inline SceneNode const *getNode(std::string const &uuid) const { return mTree.count(uuid) > 0 ? mTree.at(uuid) : nullptr; }

  private:
    std::unordered_map<std::string, SceneNode *> mTree    = {};
    std::unordered_set<SceneNode *>              mMeshes  = {};
    std::unordered_set<SceneNode *>              mLights  = {};
    std::unordered_set<SceneNode *>              mCameras = {};
};

//---

//---------------------------------------------------------------------------------------------------------------------
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
//---------------------------------------------------------------------------------------------------------------------

//=====================================
// RENDER CPU SIDE - NODES
//=====================================

//---

struct Mesh : public SceneNode
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
        glm::mat4 transform = glm::mat4(1.f);
        glm::vec4 color     = Magenta;
    };

    std::vector<uint32_t> indices   = {};
    std::vector<Vertex>   vertices  = {};
    std::vector<Instance> instances = {};

    virtual SceneNodeType getType() override { return Mesh_tc; }
};

//---

struct Light : public SceneNode
// To direct use on UBOs : Keep it aligned (only)
{
    // On Shader: struct Light { vec4 pos; vec4 color; vec4 dir; float type; float I; float angle; float blend; };

    enum Type : uint32_t
    {
        Point,
        Spot,
        Dir
    };

    glm::vec4 pos   = {AxisY * 5.f, 1.f}; // .w==0 means disabled
    glm::vec4 color = Yellow;
    glm::vec4 dir   = Zero4; // (Eurler Angles) Apply to (0,-1,0)

    float type     = (float)Point;
    float intesity = 1.f;  // [ 0.0 : 100'000 ] on Watts
    float angle    = 0.2f; // [ 0.0 : PI ]
    float blend    = 0.2f; // [ 0.0 : 1.0 ]

    virtual SceneNodeType getType() override { return Light_tc; }
};
glm::vec3 getLightFront(Light const &light)
{
    return glm::normalize(rotVecToMat4(light.dir)[2]);
}

//---

struct Camera : public SceneNode
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

    Mode mode = Mode::Orbital;
    Type type = Type::Perspective;

    virtual SceneNodeType getType() override { return Camera_tc; }
};

//---

//---------------------------------------------------------------------------------------------------------------------
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
//---------------------------------------------------------------------------------------------------------------------

//=====================================
// RENDER CPU SIDE - MATERIALS
//=====================================

//---

struct Texture
{
    //...
};

//---

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

//---

//---------------------------------------------------------------------------------------------------------------------
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
//---------------------------------------------------------------------------------------------------------------------

} // namespace dc

//---------------------------------------------------------------------------------------------------------------------
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
// XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
//---------------------------------------------------------------------------------------------------------------------

//=====================================
// MAIN
//=====================================

//---

#define TEST_N 2

//---

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
#elif TEST_N == 2
    entt::registry registry;

    /*

    How to follow the dev of this:

    1. Move all the Vulkan stuff to a "Render" folder

    2. Move general helpers like "Macros.h" and "Utils.h" to "Utils" folder

    3. Move all the Scene logic to "Scene" folder. i.e. The CPU representation of Render-Data


    Available Components
    ---
     |-> [Data] Mesh (: indices[M], vertices[N], uvs[N], normals[N], tangents[N], bitangents[N], colors[N])
     |-> [Data] Light (: type{point,spot,sun}, color, intensity, angle, blend)
     |-> [Data] Camera (: fov, perspective-type, rotation-mode | eye+lookat now are resposability of Transform-comp)
     |-> [Data] Material (: textures, props, colors)
     |-> [Data] Transform (: pos, scl, rot, matrix())
     |-> [Render] (: type{mesh, quad, line, point}, source=data-mesh*, renderApiType:this is the opengl/vulkan stuff)
     |-> [Physics] RigidBody/SoftBody (: )
     |-> [Physics] Collider (: offset, size/radius, convex-mesh)
     |-> [Physics] Material (: friction, density, restitution, restitution-threshold)


    */

#endif
}

//---

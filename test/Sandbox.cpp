#include "VonkWindow.h"
#include "Vonk.h"

#include "_glm.h"

int main()
{
  vonk::Vonk vonk;
  // mVulkan = vonk::init(1280, 720, "Vonk", /*...callbacks*/);  // @DANI

  //=============================================================================

  // === Window

  //-------------------------------------

  vonk::window::init(1280, 720, "Vonk");

  //-------------------------------------

  vonk::window::setUserPointer(&vonk);

  //-------------------------------------

  vonk::window::setCallbackKeyboard(
    [](MBU GLFWwindow *windowHandle, int key, MBU int scancode, int action, MBU int mods) {
      auto *vonk = static_cast<vonk::Vonk *>(vonk::window::userPtr);  // Cast UserPtr before use it.
      if (key == GLFW_KEY_1 and action == GLFW_PRESS) { vonk->iterScenes(); }
    });

  //-------------------------------------

  //=============================================================================

  // === Init

  vonk.init();
  MBU auto const &ds_hardcode   = vonk.createDrawShader("base", "base", "base");
  MBU auto const &ds_vertexdata = vonk.createDrawShader("withInputData", "base_2", "base_2");

  //=============================================================================

  // === Setup

  //-------------------------------------

  // ::: Scene 1

  vonk::DrawPipelineData_t const pipelineCI1 {
    .useMeshes          = false,
    .pDrawShader        = &ds_hardcode,
    .commandBuffersData = { { .commands = [](VkCommandBuffer cb) { vkCmdDraw(cb, 6, 1, 0, 0); } } },
  };

  //-------------------------------------

  // ::: Scene 2

  float constexpr meshSize = 0.5f;
  auto const dataScene2    = vonk.createMesh(
    // Indices
    // { 0, 1, 2, 2, 3, 0 },  // CW
    { 0, 3, 2, 0, 2, 1 },  // CCW
    // Vertices
    {
      { .vertex = { -meshSize, meshSize, 0.0f }, .color = { 1, 0, 0 } },   // 0
      { .vertex = { meshSize, meshSize, 0.0f }, .color = { 0, 1, 0 } },    // 1
      { .vertex = { meshSize, -meshSize, 0.0f }, .color = { 0, 0, 1 } },   // 2
      { .vertex = { -meshSize, -meshSize, 0.0f }, .color = { 1, 1, 1 } },  // 3
    });

  vonk::DrawPipelineData_t const pipelineCI2 {
    .useMeshes          = true,
    .pDrawShader        = &ds_vertexdata,
    .commandBuffersData = { { .commands = [&](VkCommandBuffer cb) { vonk.drawMesh(cb, dataScene2); } } },
  };

  //-------------------------------------

  // ::: Scene 3 (from assimp)

  auto const                     dataScene3 = vonk.read3DFile("./assets/meshes/sphere.glb");
  vonk::DrawPipelineData_t const pipelineCI3 {
    .useMeshes          = true,
    .pDrawShader        = &ds_vertexdata,
    .commandBuffersData = { { .commands = [&](VkCommandBuffer cb) { vonk.drawMeshes(cb, dataScene3); } } },
  };

  //-------------------------------------

  // ::: Add pipelines to the manager

  vonk.addPipeline(pipelineCI1);
  vonk.addPipeline(pipelineCI2);
  vonk.addPipeline(pipelineCI3);

  //-------------------------------------

  //=============================================================================

  // === Loop

  vonk::window::loop(
    [&]() { vonk.drawFrame(); },   //
    [&]() { vonk.waitDevice(); },  //
    false                          //
  );

  //=============================================================================

  // === Clean up

  vonk.cleanup();

  //=============================================================================
}

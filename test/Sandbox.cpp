#include "VonkWindow.h"
#include "Vonk.h"

#include "_glm.h"

int main()
{
  vonk::Vonk vonk;
  // mVulkan = vonk::init(1280, 720, "Vonk", /*...callbacks*/);

  // .
  // . Window

  vonk::window::init(1280, 720, "Vonk");

  vonk::window::setUserPointer(&vonk);

  vonk::window::setCallbackKeyboard(
    [](MBU GLFWwindow *windowHandle, int key, MBU int scancode, int action, MBU int mods) {
      auto *vonk = static_cast<vonk::Vonk *>(vonk::window::userPtr);  // Cast UserPtr before use it.

      if (key == GLFW_KEY_1 and action == GLFW_PRESS) { vonk->iterScenes(); }
    });

  // .
  // . Init

  vonk.init();
  MBU auto const &baseDrawShader = vonk.createDrawShader("base", "base", "base");
  MBU auto const &altDrawShader  = vonk.createDrawShader("withInputData", "base_2", "base");

  // .
  // . Setup

  // ::: Scene 1
  vonk::DrawPipelineData_t const pipelineCI {
    .useMeshes          = false,
    .pDrawShader        = &baseDrawShader,
    .commandBuffersData = { { .commands = [](VkCommandBuffer cb) { vkCmdDraw(cb, 6, 1, 0, 0); } } },
  };

  // ::: Scene 2
  auto const mesh = vonk.createMesh({ { { 0.0f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
                                      { { 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
                                      { { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } } });

  vonk::DrawPipelineData_t pipelineCI2;
  pipelineCI2.useMeshes          = true;
  pipelineCI2.pDrawShader        = &altDrawShader;
  pipelineCI2.commandBuffersData = { { .commands = [&](VkCommandBuffer cb) { vonk.drawMesh(cb, mesh); } } };

  vonk.addPipeline(pipelineCI);
  vonk.addPipeline(pipelineCI2);

  // .
  // . Loop

  vonk::window::loop(
    [&]() { vonk.drawFrame(); },   //
    [&]() { vonk.waitDevice(); },  //
    false                          //
  );

  // .
  // . Clean up

  vonk.cleanup();
}

#include "VonkWindow.h"
#include "VonkBase.h"
#include "VonkCreate.h"

int main()
{
  vonk::Base mVulkan;
  // mVulkan = vonk::init(1280, 720, "Vonsai", /*...callbacks*/);

  // .
  // . Window

  vonk::window::init(1280, 720, "Vonsai");

  vonk::window::setUserPointer(&mVulkan);

  vonk::window::setCallbackKeyboard(
    [](MBU GLFWwindow *windowHandle, int key, MBU int scancode, int action, MBU int mods) {
      auto *vonk = static_cast<vonk::Base *>(vonk::window::userPtr);  // Cast UserPtr before use it.

      if (key == GLFW_KEY_1 and action == GLFW_PRESS) { vonk->iterScenes(); }
    });

  // .
  // . Init

  mVulkan.init();

  // .
  // . Setup

  // const std::vector<Vertex> vertices = { { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
  //                                        { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
  //                                        { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } } };

  vonk::PipelineData_t const pipelineCI {
    // Static
    .shadersData = { { "base", VK_SHADER_STAGE_VERTEX_BIT }, { "base", VK_SHADER_STAGE_FRAGMENT_BIT } },
    // Dynamic
    .commandBuffersData = { { .commands = [](VkCommandBuffer cb) { vkCmdDraw(cb, 3, 1, 0, 0); } } },
  };

  vonk::PipelineData_t pipelineCI2 = pipelineCI;
  pipelineCI2.shadersData[0]       = { "base2", VK_SHADER_STAGE_VERTEX_BIT },
  pipelineCI2.commandBuffersData   = { { .commands = [](VkCommandBuffer cb) { vkCmdDraw(cb, 6, 1, 0, 0); } } },

  mVulkan.addPipeline(pipelineCI);
  // mVulkan.addPipeline(pipelineCI2);

  // .
  // . Loop

  vonk::window::loop(
    [&]() { mVulkan.drawFrame(); },   //
    [&]() { mVulkan.waitDevice(); },  //
    false                             //
  );

  // .
  // . Clean up

  mVulkan.cleanup();
}

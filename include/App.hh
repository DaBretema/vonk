#pragma once

#include <algorithm>
#include <string>

#include <GLFW/glfw3.h>
#include "VkToolbox.hh"

namespace Vonsai
{
class App
{
public:
  App(int w, int h, const char *title) : mW(w), mH(h), mTitle(title) {}
  void run();

private:
  int         mW { 800 };
  int         mH { 600 };
  std::string mTitle { "Vonsai!" };

  vk::UniqueInstance mInstance;
  GLFWwindow *       mWindow { nullptr };

  // App flow
  void initWindow();
  void initVulkan();
  void mainLoop();
  void cleanup();

  // To init vulkan...
  void createInstance();

  // Get vulkan data (layers, extensions, ...)
  bool                      checkValidationLayerSupport();
  std::vector<const char *> getRequiredExtensions();
  // std::tuple<uint32_t, const char **> getRequiredExtensions();
};

}  // namespace Vonsai

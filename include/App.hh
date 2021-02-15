#pragma once

#include <GLFW/glfw3.h>
#include <algorithm>
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

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

  void initWindow();
  void initVulkan();
  void mainLoop();
  void cleanup();

  void createInstance();

  bool checkValidationLayerSupport();
};

}  // namespace Vonsai

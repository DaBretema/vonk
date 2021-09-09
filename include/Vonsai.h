#pragma once

#include <string>
#include <utility>
#include <vector>

#define VK_USE_PLATFORM_MACOS_MVK
#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

namespace vo
{
class Vonsai
{
public:
  Vonsai(int w, int h, std::string title) : mW(w), mH(h), mTitle(std::move(title)) {}
  void run();

private:


  // App variables
  //---------------------------------------------------------------------------

  int mW { 800 };
  int mH { 600 };
  std::string mTitle { "Vonsai!" };

  GLFWwindow *mWindow { nullptr };


  // App flow
  //---------------------------------------------------------------------------

  void initWindow();
  void initVulkan();
  void mainLoop();
  void cleanup();


  // Vulkan variables
  //---------------------------------------------------------------------------

  VkInstance mVkInstance{};


  // Vulkan initialization
  //---------------------------------------------------------------------------

  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSwapChain();

};

}  // namespace Vonsai

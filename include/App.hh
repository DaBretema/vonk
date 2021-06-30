#pragma once

#include <string>
#include <vector>

#include "vk0/vk0.hh"
#include "vk0/vk0Debug.hh"
#include "vk0/vk0QueueFamilyIndices.hh"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

namespace Vonsai
{
class App
{
public:
  App(int w, int h, const char *title) : mW(w), mH(h), mTitle(title) {}
  void run();

private:
  //---------------------------------------------------------------------------

  int mW { 800 };
  int mH { 600 };

  std::string mTitle { "Vonsai!" };

  GLFWwindow *mWindow { nullptr };

  //---------------------------------------------------------------------------

  vk::UniqueInstance mInstance;

  vk::PhysicalDevice mPhysicalDevice;
  vk::UniqueDevice   mLogicalDevice;

  vk::Queue               mQueueGraphics;
  vk::Queue               mQueuePresent;
  vk0::QueueFamilyIndices mQueueFamilyIndices;

  vk::SurfaceKHR   mSurface;

  vk::SwapchainKHR swapChain;
  std::vector<vk::Image> swapChainImages;
  vk::Format swapChainImageFormat;
  vk::Extent2D swapChainExtent;

  vk0::DebugMessenger mDebugMessenger;

  //---------------------------------------------------------------------------

  // App flow
  void initWindow();
  void initVulkan();
  void mainLoop();
  void cleanup();

  //---------------------------------------------------------------------------

  // To init vulkan...
  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSwapChain();

  //---------------------------------------------------------------------------

  // Get vulkan data (layers, extensions, ...)
  bool                      checkValidationLayerSupport();
  std::vector<const char *> getRequiredExtensions();

  //---------------------------------------------------------------------------
};

}  // namespace Vonsai

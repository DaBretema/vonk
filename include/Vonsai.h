#pragma once

#include <string>
#include <vector>

#include "VwVulkan.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "VwQueueFamily.h"
#include "VwDebugMessenger.h"
#include "VwSwapChain.h"

namespace vo
{
//===============
//===============
//===============
//===============
//===============

// VONSAI

class Vonsai
{
public:
  Vonsai(int w, int h, std::string title) : mW(w), mH(h), mTitle(std::move(title)) {}
  void run();

private:
  //

  // ::: App variables
  int         mW { 800 };
  int         mH { 600 };
  std::string mTitle { "Vonsai!" };

  GLFWwindow *mWindow { nullptr };

  // ::: App flow

  void initWindow();
  void initVulkan();
  void mainLoop();
  void cleanup();

  // ::: Vulkan variables

  VkInstance mInstance = VK_NULL_HANDLE;

  VkPhysicalDevice           mPhysicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceFeatures   mPhysicalDeviceFeatures;
  VkPhysicalDeviceProperties mPhysicalDeviceProperties;

  VkDevice mLogicalDevice = VK_NULL_HANDLE;

  vo::QueueFamily    mQueueFamilies {};
  vo::DebugMessenger mDebugMessenger {};

  VkSurfaceKHR mSurface;

  VkSwapchainKHR           mSwapChain;
  std::vector<VkImage>     mSwapChainImages;
  vo::SwapChainSettings    mSwapChainSettings;
  std::vector<VkImageView> mSwapChainImageViews;

  // ::: Vulkan initialization

  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSwapChain();
  void createImageViews();
  void createGraphicsPipeline();
};

}  // namespace vo

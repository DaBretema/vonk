#pragma once

#include <string>
#include <utility>
#include <vector>

#include "vk0/vk0.hh"
#include "vk0/vk0Debug.hh"
#include "vk0/vk0QueueFamilyIndices.hh"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

namespace vonsai
{
class app
{
public:
  app(int w, int h, std::string title) : mW(w), mH(h), mTitle(std::move(title)) {}
  void run();

private:


  // App variables
  //---------------------------------------------------------------------------

  int mW { 800 };
  int mH { 600 };
  std::string mTitle { "vonsai!" };

  GLFWwindow *mWindow { nullptr };


  // App flow
  //---------------------------------------------------------------------------

  void initWindow();
  void initVulkan();
  void mainLoop();
  void cleanup();


  // Vulkan variables
  //---------------------------------------------------------------------------

  vk::UniqueInstance mInstance;

  vk::PhysicalDevice mPhysicalDevice;
  vk::UniqueDevice   mLogicalDevice;

  vk::Queue mQueueGraphics;
  vk::Queue mQueuePresent;
  vk0::QueueFamilyIndices    mQueueFamilyIndices;

  vk::SurfaceKHR   mSurface;

  vk::SwapchainKHR swapChain;
  std::vector<vk::Image> swapChainImages;
  vk::Format swapChainImageFormat;
  vk::Extent2D swapChainExtent;

  vk0::DebugMessenger mDebugMessenger;


  // Vulkan initialization
  //---------------------------------------------------------------------------

  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSwapChain();

};

}  // namespace vonsai

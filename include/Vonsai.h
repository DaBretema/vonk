#pragma once

#include <string>
#include <vector>

#include "VoWindow.h"
#include "VoVulkanDevice.h"

namespace vo
{  //

class Vonsai
{
public:
  Vonsai(uint32_t w, uint32_t h, std::string title) : mW(w), mH(h), mTitle(std::move(title)) {}
  void run();

private:
  // ::: Vulkan variables
  vku::Instance mInstance;
  vku::Device   mDevice;

  // ::: App variables
  uint32_t    mW { 800 };
  uint32_t    mH { 600 };
  std::string mTitle { "Vonsai!" };

  // ::: Flow
  void initVulkan();
  void mainLoop();
  void drawFrame();
  void cleanup();

  //...

  // void createInstance();
  // void createSurface();
  // void pickPhysicalDevice();
  // void createLogicalDevice();
  // void createSwapChain();
  // void createImageViews();
  // void createRenderPass();
  // void createShaders();
  // void createGraphicsPipeline();
  // void createFramebuffers();
  // void createCommandPool();
  // void createCommandBuffers();
  // void createSyncObjects();
  // void recreateSwapChain();
  // void cleanupSwapChain();

  //...
};

}  // namespace vo

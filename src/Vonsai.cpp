#include "Vonsai.h"

#include <fmt/core.h>
#include "Settings.h"

#ifndef NDEBUG
#  define VO_TRACE(lvl, msg) fmt::print("{} {}\n", std::string(lvl, '>'), msg)
#else
#  define VO_TRACE(s)
#endif

#define VO_CHECK(vulkanCode) \
  if (vulkanCode != VK_SUCCESS) { \
    fmt::print("[VK_ERR] - {}:{}\n↪ {}", __FILE__, __LINE__,#vulkanCode); \
    exit(-1); \
  }


//-----------------------------------------------------------------------------
// === HELPERS
//-----------------------------------------------------------------------------

std::vector<char const *> sGetRequiredExtensions()
{
  uint32_t     glfwExtensionCount = 0;
  char const ** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<char const *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (vo::sHasLayers)
    extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  return extensions;
}

//-----------------------------------------------------------------------------

bool sCheckValidationLayerSupport()
{
#if 0
  auto const availableLayers = vk::enumerateInstanceLayerProperties().value;

  for (auto layerName : vk0::layers) {
    auto const found = std::any_of(availableLayers.begin(), availableLayers.end(), [&](auto const layer) {
      return std::string_view(layerName) == layer.layerName;
    });

    if (!found) {
      vk0AlertFmt("Layer {} not found.", layerName);
      return false;
    }
  }
  return true;
#else
  return false;
#endif
}

//===============================================
//===============================================
//===============================================


namespace vo
{

//-----------------------------------------------------------------------------
// === ENTRY POINT
//-----------------------------------------------------------------------------

void Vonsai::run()
{
  VO_TRACE(1, "INIT WINDOW");
  initWindow();
  VO_TRACE(1, "INIT VULKAN");
  initVulkan();
  VO_TRACE(1, "MAIN LOOP");
  mainLoop();
  VO_TRACE(1, "CLEAN UP");
  cleanup();
}


//-----------------------------------------------------------------------------
// === TO RUN
//-----------------------------------------------------------------------------

void Vonsai::initWindow()
{
  VO_TRACE(2, "GLFW Init");
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Avoid OpenGL context creation
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // Resize windows takes special care

  VO_TRACE(2, "GLFW Create Window");
  mWindow = glfwCreateWindow(mW, mH, mTitle.c_str(), nullptr, nullptr);
}

//-----------------------------------------------------------------------------

void Vonsai::initVulkan()
{
  VO_TRACE(2, "Create Instance");
  createInstance();
//  VO_TRACE(2, "Setup Debug callback");
//  mDebugMessenger.create(*mInstance);
//  VO_TRACE(2, "Create Surface");
//  createSurface();
//  VO_TRACE(2, "Pick physical device");
//  pickPhysicalDevice();
//  VO_TRACE(2, "Create logical device");
//  createLogicalDevice();
//  VO_TRACE(2, "Create SWAP-CHAIN");
//  createSwapChain();
}

//-----------------------------------------------------------------------------

void Vonsai::mainLoop()
{
  while (!glfwWindowShouldClose(mWindow)) {
    glfwWaitEvents();
    // glfwPollEvents();
  }
}

//-----------------------------------------------------------------------------

void Vonsai::cleanup()
{
  vkDestroyInstance(mVkInstance, nullptr);
//  // NOTE: mInstance and mDevice destruction is handled by Unique..wrappers
//
//  // not using UniqeSwapchain to destroy in correct order - before the surface
//  mLogicalDevice->destroySwapchainKHR(swapChain);
//
//  // NOTE: Surface is created by glfw, therefore not using a Unique handle
//  mInstance->destroySurfaceKHR(mSurface);
//
//  // NOTE: Comment line below to verify that VALIDATION LAYERS are working...
//  mDebugMessenger.destroy(*mInstance);

  glfwDestroyWindow(mWindow);
  glfwTerminate();
}


//-----------------------------------------------------------------------------
// === TO INIT VULKAN
//-----------------------------------------------------------------------------

void Vonsai::createInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = mTitle.c_str();
    appInfo.applicationVersion = VK_API_VERSION_1_2;
    appInfo.pEngineName        = (mTitle+" Engine").c_str();
    appInfo.engineVersion      = VK_API_VERSION_1_2;
    appInfo.apiVersion         = VK_API_VERSION_1_2;


    VkInstanceCreateInfo createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // . Extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    createInfo.enabledExtensionCount   = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    // . Layers
    createInfo.enabledLayerCount = 0;

    VO_CHECK(vkCreateInstance(&createInfo, nullptr, &mVkInstance));
}

//-----------------------------------------------------------------------------

void Vonsai::createSurface()
{
//  VkSurfaceKHR rawSurface;
//  vk0TestFnC(glfwCreateWindowSurface(*mInstance, mWindow, nullptr, &rawSurface))
//
//  mSurface = rawSurface;
}

//-----------------------------------------------------------------------------

void Vonsai::pickPhysicalDevice()
{
//  auto const devices = vk0Call(mInstance->enumeratePhysicalDevices());
//  if (devices.empty()) { vk0AlertAbort("Failed to find GPUs with Vulkan support!") }
//
//  for (const auto &device : devices) {
//    auto const queueFamilyIndices = vk0::QueueFamilyIndices { device, mSurface };
//
//    if (queueFamilyIndices.isComplete()) {
//      mQueueFamilyIndices = queueFamilyIndices;
//      mPhysicalDevice     = device;
//      break;
//    }
//  }
//  if (!mPhysicalDevice) { vk0AlertAbort("Failed to find a suitable GPU") }
//
//#if vk0VERBOSE
//  // . Getting extensions
//  auto const extensions = vk0Call(mPhysicalDevice.enumerateDeviceExtensionProperties());
//  // . Show info about extensions
//  fmt::print("[*] Available DEVICE Extensions [{}]\n", fmt::ptr(&mPhysicalDevice));
//  for (auto const &ext : extensions) { fmt::print(" ↳ {}\n", ext.extensionName); }
//#endif
}

//-----------------------------------------------------------------------------

void Vonsai::createLogicalDevice()
{
//  auto const deviceFeatures   = vk::PhysicalDeviceFeatures();
//  auto const queuesCreateInfo = mQueueFamilyIndices.getCreateInfo();
//  auto const createInfo       = vk::DeviceCreateInfo { vk::DeviceCreateFlags(),
//                                                 vk0SizeData(queuesCreateInfo),
//                                                 vk0SizeData(vk0::layers),
//                                                 vk0SizeData(vk0::extensions),
//                                                 &deviceFeatures };
//
//  mLogicalDevice = vk0Call(mPhysicalDevice.createDeviceUnique(createInfo));
//  mQueueGraphics = mLogicalDevice->getQueue(mQueueFamilyIndices.graphicsFamily.value(), 0);
//  mQueuePresent  = mLogicalDevice->getQueue(mQueueFamilyIndices.presentFamily.value(), 0);
}

//-----------------------------------------------------------------------------

void Vonsai::createSwapChain() {}

//-----------------------------------------------------------------------------

}  // namespace Vonsai

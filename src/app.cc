#include "app.hh"

// #include <string>
#include <tuple>
#include <algorithm>

#include <fmt/core.h>
//#include <fmt/format.h>

#include "vk0/vk0Macros.hh"
#include "vk0/vk0Settings.hh"
#include "vk0/vk0Debug.hh"

#ifndef NDEBUG
#  define VO_TRACE(lvl, msg) fmt::print("{} {}\n", std::string(lvl, '>'), msg)
#else
#  define VO_TRACE(s)
#endif


//-----------------------------------------------------------------------------
// === HELPERS
//-----------------------------------------------------------------------------

std::vector<const char *> sGetRequiredExtensions()
{
  uint32_t     glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (vk0::hasLayers) { extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

  return extensions;
}

//-----------------------------------------------------------------------------

bool sCheckValidationLayerSupport()
{
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
}

//===============================================
//===============================================
//===============================================


namespace vonsai
{

//-----------------------------------------------------------------------------
// === ENTRY POINT
//-----------------------------------------------------------------------------

void app::run()
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

void app::initWindow()
{
  VO_TRACE(2, "GLFW Init");
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Avoid OpenGL context creation
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // Resize windows takes special care

  VO_TRACE(2, "GLFW Create Window");
  mWindow = glfwCreateWindow(mW, mH, mTitle.c_str(), nullptr, nullptr);
}

//-----------------------------------------------------------------------------

void app::initVulkan()
{
  VO_TRACE(2, "Create Instance");
  createInstance();
  VO_TRACE(2, "Setup Debug callback");
  mDebugMessenger.create(*mInstance);
  VO_TRACE(2, "Create Surface");
  createSurface();
  VO_TRACE(2, "Pick physical device");
  pickPhysicalDevice();
  VO_TRACE(2, "Create logical device");
  createLogicalDevice();
  VO_TRACE(2, "Create SWAP-CHAIN");
  createSwapChain();
}

//-----------------------------------------------------------------------------

void app::mainLoop()
{
  while (!glfwWindowShouldClose(mWindow)) {
    glfwWaitEvents();  // glfwPollEvents();
  }
}

//-----------------------------------------------------------------------------

void app::cleanup()
{
  // NOTE: mInstance and mDevice destruction is handled by Unique..wrappers

  // not using UniqeSwapchain to destroy in correct order - before the surface
  mLogicalDevice->destroySwapchainKHR(swapChain);

  // NOTE: Surface is created by glfw, therefore not using a Unique handle
  mInstance->destroySurfaceKHR(mSurface);

  // NOTE: Comment line below to verify that VALIDATION LAYERS are working...
  mDebugMessenger.destroy(*mInstance);

  glfwDestroyWindow(mWindow);
  glfwTerminate();
}


//-----------------------------------------------------------------------------
// === TO INIT VULKAN
//-----------------------------------------------------------------------------

void app::createInstance()
{
  if (vk0::hasLayers && !sCheckValidationLayerSupport()) {
    VO_TRACE(3, "Checking validation layers");
    vk0Alert("Validation layers NOT available!");
  }

  auto constexpr v   = VK_API_VERSION_1_0;
  auto const appInfo = vk::ApplicationInfo { mTitle.c_str(), v, "vonsai", v, v };

  // . Window Extensions
  auto const exts = sGetRequiredExtensions();

  // . Creating instance
  auto const flags = vk::InstanceCreateFlags();
  auto const info  = vk::InstanceCreateInfo { flags, &appInfo, vk0SizeData(vk0::layers), vk0SizeData(exts) };

  mInstance = vk0Call(vk::createInstanceUnique(info, nullptr));

#if vk0Verbose
  // . Getting extensions
  auto const extensions = vk0Call(vk::enumerateInstanceExtensionProperties());
  // . Show info about extensions
  fmt::print("[*] Available INSTANCE Extensions\n");
  for (auto const &ext : extensions) { fmt::print(" ↳ {}\n", ext.extensionName); }
  for (auto const &ext : extensions) { fmt::print(" ↳ {}\n", ext.extensionName); }
#endif
}

//-----------------------------------------------------------------------------

void app::createSurface()
{
  VkSurfaceKHR rawSurface;
  vk0TestFnC(glfwCreateWindowSurface(*mInstance, mWindow, nullptr, &rawSurface))

  mSurface = rawSurface;
}

//-----------------------------------------------------------------------------

void app::pickPhysicalDevice()
{
  auto const devices = vk0Call(mInstance->enumeratePhysicalDevices());
  if (devices.empty()) { vk0AlertAbort("Failed to find GPUs with Vulkan support!") }

  for (const auto &device : devices) {
    auto const queueFamilyIndices = vk0::QueueFamilyIndices { device, mSurface };

    if (queueFamilyIndices.isComplete()) {
      mQueueFamilyIndices = queueFamilyIndices;
      mPhysicalDevice     = device;
      break;
    }
  }
  if (!mPhysicalDevice) { vk0AlertAbort("Failed to find a suitable GPU") }

#if vk0VERBOSE
  // . Getting extensions
  auto const extensions = vk0Call(mPhysicalDevice.enumerateDeviceExtensionProperties());
  // . Show info about extensions
  fmt::print("[*] Available DEVICE Extensions [{}]\n", fmt::ptr(&mPhysicalDevice));
  for (auto const &ext : extensions) { fmt::print(" ↳ {}\n", ext.extensionName); }
#endif
}

//-----------------------------------------------------------------------------

void app::createLogicalDevice()
{
  auto const deviceFeatures   = vk::PhysicalDeviceFeatures();
  auto const queuesCreateInfo = mQueueFamilyIndices.getCreateInfo();
  auto const createInfo       = vk::DeviceCreateInfo { vk::DeviceCreateFlags(),
                                                 vk0SizeData(queuesCreateInfo),
                                                 vk0SizeData(vk0::layers),
                                                 vk0SizeData(vk0::extensions),
                                                 &deviceFeatures };

  mLogicalDevice = vk0Call(mPhysicalDevice.createDeviceUnique(createInfo));
  mQueueGraphics = mLogicalDevice->getQueue(mQueueFamilyIndices.graphicsFamily.value(), 0);
  mQueuePresent  = mLogicalDevice->getQueue(mQueueFamilyIndices.presentFamily.value(), 0);
}

//-----------------------------------------------------------------------------

void app::createSwapChain() {}

//-----------------------------------------------------------------------------

}  // namespace vonsai

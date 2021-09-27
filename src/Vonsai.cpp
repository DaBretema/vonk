#include "Vonsai.h"

#include <map>
#include <vector>

#include <fmt/core.h>

#include "Macros.h"
#include "Settings.h"

//-----------------------------------------------------------------------------
// === HELPERS
//-----------------------------------------------------------------------------

std::vector<char const *> sGetInstanceExtensions()
{
  uint32_t     glfwExtensionCount = 0;
  char const **glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<char const *> instanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (vo::sHasValidationLayers) { instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

  if (vo::sHasInstanceExtensions) {
    for (auto &&ext : vo::sInstanceExtensions) { instanceExtensions.emplace_back(ext); }
  }

  return instanceExtensions;
}

//-----------------------------------------------------------------------------

bool sCheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
  uint32_t count;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> available(count);
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, available.data());

  std::set<std::string> required(vo::sDeviceExtensions.begin(), vo::sDeviceExtensions.end());
  for (const auto &item : available) { required.erase(item.extensionName); }
  return required.empty();
}

//-----------------------------------------------------------------------------

bool sCheckValidationLayerSupport()
{
  uint32_t count;
  vkEnumerateInstanceLayerProperties(&count, nullptr);
  std::vector<VkLayerProperties> available(count);
  vkEnumerateInstanceLayerProperties(&count, available.data());

  std::set<std::string> required(vo::sValidationLayers.begin(), vo::sValidationLayers.end());
  for (const auto &item : available) { required.erase(item.layerName); }
  return required.empty();

  // auto &L = available;
  // for (auto name : vo::sValidationLayers) {
  //   auto const found = std::any_of(L.begin(), L.end(), [&](auto &&l) { return std::string_view(name) == l.layerName;
  //   }); if (!found) {
  //     VO_ERR_FMT("Layer {} not found.", name);
  //     return false;
  //   }
  // }

  return true;
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
  VO_CHECK(glfwInit());
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // Resize windows takes special care
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Avoid OpenGL context creation

  VO_TRACE(2, "GLFW Create Window");
  mWindow = glfwCreateWindow(mW, mH, mTitle.c_str(), nullptr, nullptr);

  // [] CALLBACKS SETUP ...
}

//-----------------------------------------------------------------------------

void Vonsai::initVulkan()
{
  VO_TRACE(2, "Create Instance");
  createInstance();
  VO_TRACE(2, "Setup Debug callback");
  mDebugMessenger.create(mInstance);
  VO_TRACE(2, "Create Surface");
  createSurface();
  VO_TRACE(2, "Pick physical device");
  pickPhysicalDevice();
  VO_TRACE(2, "Create logical device");
  createLogicalDevice();
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
  // . Devices
  vkDestroyDevice(mLogicalDevice, nullptr);

  // . Clean everything related to the instance
  mDebugMessenger.destroy(mInstance);
  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

  // . Destroy the instance itself
  vkDestroyInstance(mInstance, nullptr);

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
  if (vo::sHasValidationLayers and !sCheckValidationLayerSupport()) {
    VO_ABORT("Validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo {};
  appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName   = mTitle.c_str();
  appInfo.applicationVersion = VK_API_VERSION_1_2;
  appInfo.pEngineName        = (mTitle + " Engine").c_str();
  appInfo.engineVersion      = VK_API_VERSION_1_2;
  appInfo.apiVersion         = VK_API_VERSION_1_2;

  VkInstanceCreateInfo createInfo {};
  createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  // . Esential extensions
  auto const instanceExtensions      = sGetInstanceExtensions();
  createInfo.enabledExtensionCount   = VW_SIZE_CAST(instanceExtensions.size());
  createInfo.ppEnabledExtensionNames = instanceExtensions.data();

  // . Layers
  if (vo::sHasValidationLayers) {
    createInfo.enabledLayerCount   = static_cast<uint32_t>(vo::sValidationLayers.size());
    createInfo.ppEnabledLayerNames = vo::sValidationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  VW_CHECK(vkCreateInstance(&createInfo, nullptr, &mInstance));

#if VO_VERBOSE
  // . Show all available extensions
  uint32_t allExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &allExtensionCount, nullptr);
  std::vector<VkExtensionProperties> allExtensions(allExtensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &allExtensionCount, allExtensions.data());
  // .. Show info about extensions
  fmt::print("[*] Available INSTANCE Extensions\n");
  for (auto const &ext : allExtensions) { fmt::print(" ↳ {}\n", ext.extensionName); }
#endif
}

//-----------------------------------------------------------------------------

void Vonsai::createSurface() { VW_CHECK(glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface)); }

//-----------------------------------------------------------------------------

void Vonsai::pickPhysicalDevice()
{
  // . Get num of available devices
  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);
  if (physicalDeviceCount == 0) { VO_ABORT("Failed to find GPUs with Vulkan support!"); }

  // . Collect them
  std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
  vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, physicalDevices.data());

  // . Calculate score for all available physical-devices
  uint32_t maxScore = 0;
  for (const auto &physicalDevice : physicalDevices) {
    // .. VkObjects needed
    mQueueFamilies.findIndices(physicalDevice, mSurface);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    // .. Score computation
    const int      isDiscrete        = 1000 * (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    const int      discardConditions = { !mQueueFamilies.isComplete() or sCheckDeviceExtensionSupport(physicalDevice)
                                    or !features.geometryShader };
    uint32_t const score             = discardConditions * (isDiscrete + properties.limits.maxImageDimension2D);

    // .. Store candidate
    if (score > maxScore) {
      mPhysicalDevice           = physicalDevice;
      mPhysicalDeviceFeatures   = features;
      mPhysicalDeviceProperties = properties;
      maxScore                  = score;
    }
  }

  // . Pick one based on it's score
  if (maxScore < 1) { VO_ABORT("Suitable GPU not found!"); }

  // ...

#if VO_VERBOSE
  fmt::print(
    "*** Name: {} - Type: {} - Score: {} \n",
    mPhysicalDeviceProperties.deviceName,
    mPhysicalDeviceProperties.deviceType,
    pdCandidates.rbegin()->first);

  // . Show all available extensions
  uint32_t allExtensionCount = 0;
  vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &allExtensionCount, nullptr);
  std::vector<VkExtensionProperties> allExtensions(allExtensionCount);
  vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &allExtensionCount, allExtensions.data());
  // .. Show info about extensions
  fmt::print("[*] Available DEVICE Extensions\n");
  for (auto const &ext : allExtensions) { fmt::print(" ↳ {}\n", ext.extensionName); }
#endif
}

//-----------------------------------------------------------------------------

void Vonsai::createLogicalDevice()
{
  // . Objects

  // .. [Create-Info] Graphics Queue
  float const queuePriority = 1.0f;

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  for (uint32_t queueFamily : mQueueFamilies.getUniqueIndices()) {
    VkDeviceQueueCreateInfo queueCreateInfo {};
    queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  // .. Device desired features
  VkPhysicalDeviceFeatures deviceFeatures {};

  // . [Create-Info] Logical Device
  VkDeviceCreateInfo createInfo {};
  createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos    = queueCreateInfos.data();
  createInfo.queueCreateInfoCount = VW_SIZE_CAST(queueCreateInfos.size());
  createInfo.pEnabledFeatures     = &deviceFeatures;

  // .. Set device extensions
  if (vo::sHasDeviceExtensions) {
    createInfo.enabledExtensionCount   = VW_SIZE_CAST(vo::sDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = vo::sDeviceExtensions.data();
  } else {
    createInfo.enabledExtensionCount = 0;
  }

  // .. Set device validation layers
  if (vo::sHasValidationLayers) {
    createInfo.enabledLayerCount   = VW_SIZE_CAST(vo::sValidationLayers.size());
    createInfo.ppEnabledLayerNames = vo::sValidationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  // . Creation and pick real queues from it
  VW_CHECK(vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicalDevice));
  mQueueFamilies.findQueues(mLogicalDevice);
}

//-----------------------------------------------------------------------------

void Vonsai::createSwapChain() {}

//-----------------------------------------------------------------------------

}  // namespace vo

#include "Vonsai.h"

#include <map>
#include <vector>

#include <fmt/core.h>

#include "Utils.h"
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

bool sCheckPhysicalDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
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

void Vonsai::mainLoop()
{
  while (!glfwWindowShouldClose(mWindow)) {
    glfwWaitEvents();
    // glfwPollEvents();
  }
}

//-----------------------------------------------------------------------------

void Vonsai::initVulkan()
{
  VO_TRACE(2, "Create Instance");
  createInstance();
  VO_TRACE(2, "Setup Debug callback");
  // vku::debugmessenger::create(mInstance, &??mDebugMessenger);
  mDebugMessenger.create(mInstance);
  VO_TRACE(2, "Create Surface from window");
  createSurface();
  VO_TRACE(2, "Pick Physical-Device");
  pickPhysicalDevice();
  VO_TRACE(2, "Create Logical-Device");
  createLogicalDevice();
  VO_TRACE(2, "Create SWAP-CHAIN");
  createSwapChain();
  VO_TRACE(2, "Create Image-Views");
  createImageViews();
  VO_TRACE(2, "Create Graphics-Pipeline");
  createGraphicsPipeline();
}

//-----------------------------------------------------------------------------

void Vonsai::cleanup()
{
  // . Logical Device
  // .. Dependencies
  VO_TRACE(2, "Destroy Graphics-Pipeline");
  for (auto [name, shaderModule] : mShaderModules) { vkDestroyShaderModule(mLogicalDevice, shaderModule, nullptr); }
  VO_TRACE(2, "Destroy Image-Views");
  for (auto imageView : mSwapChainImageViews) { vkDestroyImageView(mLogicalDevice, imageView, nullptr); }
  VO_TRACE(2, "Destroy SWAP-CHAIN");
  vkDestroySwapchainKHR(mLogicalDevice, mSwapChain, nullptr);
  // .. Itself
  VO_TRACE(2, "Destroy Logical-Device");
  vkDestroyDevice(mLogicalDevice, nullptr);

  // . Instance
  // .. Dependencies
  VO_TRACE(2, "Destroy Surface");
  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  VO_TRACE(2, "Destroy Debug-Callback");
  // vku::debugmessenger::destroy(mInstance, mDebugMessenger);
  mDebugMessenger.destroy(mInstance);
  // .. Itself
  VO_TRACE(2, "Destroy Instance");
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

#if VO_VERBOSE_EXTENSIONS
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
    // .. Physical data
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    // .. VkObjects needed
    // vku::queues::findIndices(physicalDevice, mSurface)
    mQueueFamilies.findIndices(physicalDevice, mSurface);
    bool const swapChainIsEmpty = vku::swapchain::isEmpty(physicalDevice, mSurface);

    // .. Score computation
    const int discardConditions = { !mQueueFamilies.isComplete()                              //
                                    or swapChainIsEmpty                                       //
                                    or !sCheckPhysicalDeviceExtensionSupport(physicalDevice)  //
                                    or !features.geometryShader };

    const int      isDiscrete = 1000 * (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    uint32_t const score      = discardConditions * (isDiscrete + properties.limits.maxImageDimension2D);

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

#if VO_VERBOSE_EXTENSIONS
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
  // . [Create-Info] Queues
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

  // . [Create-Info] Logical Device
  VkDeviceCreateInfo createInfo {};
  createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos    = queueCreateInfos.data();
  createInfo.queueCreateInfoCount = VW_SIZE_CAST(queueCreateInfos.size());
  createInfo.pEnabledFeatures     = &mPhysicalDeviceFeatures;

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

void Vonsai::createSwapChain()
{
  int iW, iH;
  glfwGetFramebufferSize(mWindow, &iW, &iH);
  uint32_t w = static_cast<uint32_t>(iW), h = static_cast<uint32_t>(iH);

  mSwapChainSettings = vku::swapchain::getSettings(mPhysicalDevice, mSurface, vku::swapchain::Settings { { w, h } });

#if VO_VERBOSE
  mSwapChainSettings.dumpInfo();
#endif

  // . [Create-Info] Swap-Chain

  VkSwapchainCreateInfoKHR createInfo {};
  createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = mSurface;

  // .. Copy settings
  createInfo.minImageCount   = mSwapChainSettings.minImageCount;
  createInfo.imageFormat     = mSwapChainSettings.surfaceFormat.format;
  createInfo.imageColorSpace = mSwapChainSettings.surfaceFormat.colorSpace;
  createInfo.imageExtent     = mSwapChainSettings.extent2D;
  createInfo.presentMode     = mSwapChainSettings.presentMode;

  // .. Always 1 unless you are developing a stereoscopic 3D application.
  createInfo.imageArrayLayers = 1;

  // .. Other options:
  // VK_IMAGE_USAGE_TRANSFER_DST_BIT for post-pro and memory-op to transfer the rendered image to a swap-chain-img
  // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT for assign Depth and Stencil to main swap-chain-img (???)
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // .. How to handle swap chain images that will be used across multiple queue families
  // VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be explicitly
  //   transferred before using it in another queue family. This option offers the best performance.
  // VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit
  //   ownership transfers.
  auto const queuefamilyindices = mQueueFamilies.getUniqueIndices();
  if (queuefamilyindices.size() > 1) {
    createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = VW_SIZE_CAST(queuefamilyindices.size());
    createInfo.pQueueFamilyIndices   = queuefamilyindices.data();
  } else {
    createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;        // Optional
    createInfo.pQueueFamilyIndices   = nullptr;  // Optional
  }

  // .. We can specify that a certain transform should be applied to images in the swap chain if it is supported
  // (supportedTransforms in capabilities), like a 90 degree clockwise rotation or horizontal flip. To specify that you
  // do not want any transformation, simply specify the current transformation.
  createInfo.preTransform = mSwapChainSettings.capabilities.currentTransform;

  // .. The {compositeAlpha} field specifies if the alpha channel should be used for blending with other windows in the
  // window system. You'll almost always want to simply ignore the alpha channel.
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  // .. If clipped member is set to VK_TRUE then that means that we don't care about the color of pixels that are
  // obscured, for example because another window is in front of them. Unless you really need to be able to read these
  // pixels back and get predictable results, you'll get the best performance by enabling clipping.
  createInfo.clipped = VK_TRUE;

  // .. It's possible that your swap chain becomes invalid or unoptimized while your application is running, for example
  // because the window was resized. In that case the swap chain actually needs to be recreated from scratch and a
  // reference to the old one must be specified in this field.
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  // . Creation
  VW_CHECK(vkCreateSwapchainKHR(mLogicalDevice, &createInfo, nullptr, &mSwapChain));

  // . Caputre images
  uint32_t imageCount;
  vkGetSwapchainImagesKHR(mLogicalDevice, mSwapChain, &imageCount, nullptr);
  mSwapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(mLogicalDevice, mSwapChain, &imageCount, mSwapChainImages.data());
}

//-----------------------------------------------------------------------------

void Vonsai::createImageViews()
{
  mSwapChainImageViews.resize(mSwapChainImages.size());

  // . Create a image-view per image

  for (size_t i = 0; i < mSwapChainImages.size(); i++) {
    // .. [Create-Info] Swap-Chain
    VkImageViewCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = mSwapChainImages[i];

    // ... The viewType and format fields specify how the image data should be interpreted. The viewType parameter
    // allows you to treat images as 1D textures, 2D textures, 3D textures and cube maps.
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format   = mSwapChainSettings.surfaceFormat.format;

    // ... How to read RGBA
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // ... The subresourceRange field describes what the image's purpose is and which part of the image should be
    // accessed. **For now** set it as color targets without any mipmapping levels or multiple layers
    createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel   = 0;  // MIP-MAPing the texture [TODO]
    createInfo.subresourceRange.levelCount     = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount     = 1;

    // .. Creation
    VW_CHECK(vkCreateImageView(mLogicalDevice, &createInfo, nullptr, &mSwapChainImageViews[i]));
  }
}

//-----------------------------------------------------------------------------

void Vonsai::createGraphicsPipeline()
{
  // . Get names
  auto const name   = "base";
  auto const vsName = VO_GET_SHADER_PATH_VERT(name);
  auto const fsName = VO_GET_SHADER_PATH_FRAG(name);

  // . Read shader content
  auto vs = vo::files::read(vsName);
  auto fs = vo::files::read(fsName);
  if (vs.empty() || fs.empty()) { VO_ERR_FMT("Failed to open shaders '{}'!", name); }

  // . Register if valid
  mShaderModules.emplace(vsName, vku::shaders::createModule(mLogicalDevice, vs));
  mShaderModules.emplace(fsName, vku::shaders::createModule(mLogicalDevice, fs));

  // . Stage for vertex-shader
  VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
  vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = mShaderModules.at(vsName);
  vertShaderStageInfo.pName  = "main";  // Entrypoint function name

  // . Stage for fragment-shader
  VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
  fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = mShaderModules.at(fsName);
  fragShaderStageInfo.pName  = "main";

  // . Store
  mPipelineShaderCreateInfos.emplace_back(vertShaderStageInfo);
  mPipelineShaderCreateInfos.emplace_back(fragShaderStageInfo);
}

//-----------------------------------------------------------------------------

}  // namespace vo

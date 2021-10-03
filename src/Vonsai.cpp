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
  VO_TRACE(2, "Create RenderPass");
  createRenderPass();
  VO_TRACE(2, "Create Graphics-Pipeline");
  createGraphicsPipeline();
}

//-----------------------------------------------------------------------------

void Vonsai::cleanup()
{
  // . Logical Device
  // .. Dependencies
  VO_TRACE(2, "Destroy Graphics-Pipeline : Pipeline Itself");
  vkDestroyPipeline(mLogicalDevice, mGraphicsPipeline, nullptr);
  VO_TRACE(2, "Destroy Graphics-Pipeline : Pipeline Layout");
  vkDestroyPipelineLayout(mLogicalDevice, mPipelineLayout, nullptr);
  VO_TRACE(2, "Destroy Graphics-Pipeline : Render Pass");
  vkDestroyRenderPass(mLogicalDevice, mRenderPass, nullptr);  // after: mPipelineLayout
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

  // . Window
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

void Vonsai::createRenderPass()
{
  // . Attachment
  VkAttachmentDescription colorAttachment {};
  colorAttachment.format  = mSwapChainSettings.surfaceFormat.format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  // VK_SAMPLE_COUNT_1_BIT for No-Multisampling
  // .. load ops
  // - VK_ATTACHMENT_LOAD_OP_LOAD      : Preserve the existing contents of the attachment.
  // - VK_ATTACHMENT_LOAD_OP_CLEAR     : Clear the values to a constant at the start.
  // - VK_ATTACHMENT_LOAD_OP_DONT_CARE : Existing contents are undefined; we don't care about them.
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  // .. store ops
  // - VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later.
  // - VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation.
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  // .. stencil
  colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // .. init and final
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // . Subpass

  // .. Every subpass references one or more of the attachments that we've described using the structure in the previous
  // sections. These references are themselves VkAttachmentReference
  VkAttachmentReference colorAttachmentRef {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // .. Subpass description
  VkSubpassDescription subpass {};
  subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;  // also could be: Compute or Raytracing
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments    = &colorAttachmentRef;
  /*
  The following other types of attachments can be referenced by a subpass:
  - pInputAttachments: Attachments that are read from a shader
  - pResolveAttachments: Attachments used for multisampling color attachments
  - pDepthStencilAttachment: Attachment for depth and stencil data
  - pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved
  */

  VkRenderPassCreateInfo renderPassInfo {};
  renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments    = &colorAttachment;
  renderPassInfo.subpassCount    = 1;
  renderPassInfo.pSubpasses      = &subpass;

  VW_CHECK(vkCreateRenderPass(mLogicalDevice, &renderPassInfo, nullptr, &mRenderPass));
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

  // . Register modules if valid
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

  // . Store stages
  mPipelineShaderStageCreateInfos.emplace_back(vertShaderStageInfo);
  mPipelineShaderStageCreateInfos.emplace_back(fragShaderStageInfo);

  // . Input-State vertex
  VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
  vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount   = 0;
  vertexInputInfo.pVertexBindingDescriptions      = nullptr;  // Optional
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions    = nullptr;  // Optional

  // . Input-Assembly
  VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
  inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // . Viewport

  // .. Base
  VkViewport viewport {};
  viewport.x        = 0.0f;
  viewport.y        = 0.0f;
  viewport.width    = (float)mSwapChainSettings.extent2D.width;
  viewport.height   = (float)mSwapChainSettings.extent2D.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // .. Scissor
  VkRect2D scissor {};
  scissor.offset = { 0, 0 };
  scissor.extent = mSwapChainSettings.extent2D;

  // .. CreateInfo
  VkPipelineViewportStateCreateInfo viewportState {};
  viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports    = &viewport;
  viewportState.scissorCount  = 1;
  viewportState.pScissors     = &scissor;

  // . Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  // or _LINE / _POINT as always
  rasterizer.cullMode    = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace   = VK_FRONT_FACE_CLOCKWISE;

  // .. Disable any output
  rasterizer.rasterizerDiscardEnable = VK_FALSE;

  // .. Usefull for Shadow-Maps
  rasterizer.depthClampEnable        = VK_FALSE;
  rasterizer.depthBiasEnable         = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
  rasterizer.depthBiasClamp          = 0.0f;  // Optional
  rasterizer.depthBiasSlopeFactor    = 0.0f;  // Optional

  // .. The maximum line width that is supported depends on the hardware and any line thicker than 1.0f requires you to
  // enable the {wideLines} GPU feature.
  rasterizer.lineWidth = 1.0f;

  // . Multisampling : for now disabled
  VkPipelineMultisampleStateCreateInfo multisampling {};
  multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable   = VK_FALSE;
  multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading      = 1.0f;      // Optional
  multisampling.pSampleMask           = nullptr;   // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
  multisampling.alphaToOneEnable      = VK_FALSE;  // Optional

  // . Depth / Stencil
  VkPipelineDepthStencilStateCreateInfo depthStencil {};
  depthStencil.sType             = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.depthWriteEnable  = VK_TRUE;
  depthStencil.depthTestEnable   = VK_TRUE;
  depthStencil.depthCompareOp    = VkCompareOp::VK_COMPARE_OP_LESS;

  // . Blending

  // .. Attacment state
  VkPipelineColorBlendAttachmentState colorBlendAttachment {};
  colorBlendAttachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  // ... Color pass through
  colorBlendAttachment.blendEnable         = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;       // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;       // Optional
  // ... Classic alpha blending
  // colorBlendAttachment.blendEnable         = VK_TRUE;
  // colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  // colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  // colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
  // colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  // colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  // colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

  // .. Create info
  VkPipelineColorBlendStateCreateInfo colorBlending {};
  colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable     = VK_FALSE;
  colorBlending.logicOp           = VK_LOGIC_OP_COPY;  // Optional
  colorBlending.attachmentCount   = 1;
  colorBlending.pAttachments      = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;  // Optional
  colorBlending.blendConstants[1] = 0.0f;  // Optional
  colorBlending.blendConstants[2] = 0.0f;  // Optional
  colorBlending.blendConstants[3] = 0.0f;  // Optional

  // . Dynamic state
  VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };

  VkPipelineDynamicStateCreateInfo dynamicState {};
  dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates    = dynamicStates;

  // . Layout
  VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
  pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount         = 0;        // Optional
  pipelineLayoutInfo.pSetLayouts            = nullptr;  // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0;        // Optional
  pipelineLayoutInfo.pPushConstantRanges    = nullptr;  // Optional

  VW_CHECK(vkCreatePipelineLayout(mLogicalDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout));

  // . GRAPHCIS PIPELINE
  VkGraphicsPipelineCreateInfo pipelineInfo {};
  pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount          = VW_SIZE_CAST(mPipelineShaderStageCreateInfos.size());
  pipelineInfo.pStages             = mPipelineShaderStageCreateInfos.data();
  pipelineInfo.pVertexInputState   = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState      = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState   = &multisampling;
  pipelineInfo.pDepthStencilState  = &depthStencil;  // Optional
  pipelineInfo.pColorBlendState    = &colorBlending;
  pipelineInfo.pDynamicState       = &dynamicState;  // Optional
  pipelineInfo.layout              = mPipelineLayout;
  pipelineInfo.renderPass          = mRenderPass;
  pipelineInfo.subpass             = 0;               // index of subpass (or first subpass, not sure yet...)
  pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;  // Optional
  pipelineInfo.basePipelineIndex   = -1;              // Optional

  VW_CHECK(vkCreateGraphicsPipelines(mLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline));
}

//-----------------------------------------------------------------------------

}  // namespace vo

#include "VoVulkanUtils.h"
#include "VoVulkanToStr.h"

#include "Utils.h"

namespace vo::vulkan
{  //

//

//=============================================================================
// ---- Others ----
//=============================================================================
namespace others
{  //

  //-----------------------------------------------

  std::vector<char const *> getInstanceExtensions()
  {
    auto exts = vo::window::getRequiredInstanceExtensions();

    if (vo::sHasValidationLayers) { exts.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

    if (vo::sHasInstanceExtensions) {
      for (auto &&ext : vo::sInstanceExtensions) { exts.emplace_back(ext); }
    }

    return exts;
  }

  //-----------------------------------------------

  bool checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice, std::vector<char const *> const &exts)
  {
    if (exts.empty()) return true;

    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, available.data());

    std::set<std::string> required(exts.begin(), exts.end());
    for (const auto &item : available) { required.erase(item.extensionName); }
    return required.empty();
  }

  //-----------------------------------------------

  bool checkValidationLayersSupport(std::vector<char const *> const &layers)
  {
    if (layers.empty()) return true;

    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> available(count);
    vkEnumerateInstanceLayerProperties(&count, available.data());

    std::set<std::string> required(layers.begin(), layers.end());
    for (const auto &item : available) { required.erase(item.layerName); }
    return required.empty();
  }

}  // namespace others

//=============================================================================

//=============================================================================
// ---- Debug Messenger ----
//=============================================================================

namespace debugmessenger
{  //

  static inline VKAPI_ATTR VkBool32 VKAPI_CALL sDebugMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    MBU void *                                  pUserData)
  {
    static std::unordered_map<int32_t, bool> CACHE {};
    auto const                               ID = pCallbackData->messageIdNumber;

    if (!CACHE[ID] && messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
      fmt::print(
        "\n ❗️ [VALIDATION LAYERS] - {} at {}{}"
        "\n--------------------------------------------------------------------------------\n{}\n",
        vku::ToStr_DebugSeverity.at(messageSeverity),
        vku::ToStr_DebugType.at(messageType),
        pUserData ? fmt::format("- {}", pUserData) : std::string { "" },
        pCallbackData->pMessage);

      CACHE[ID] = true;
    }

    return VK_FALSE;
  }

  //-----------------------------------------------

  void create(VkInstance const &instance, VkDebugUtilsMessengerEXT &debugHandle)
  {
    static VkDebugUtilsMessengerCreateInfoEXT debugmessengerCreateInfo {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,

      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT    //
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT  //
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,

      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT       //
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  //
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,

      .pfnUserCallback = sDebugMessengerCallback,

      .pUserData = nullptr,  // Optional
      .pNext     = nullptr,  // Mandatory
      .flags     = 0,        // Mandatory
    };

    if (!vo::sHasValidationLayers) return;
    vku__instanceFn(instance, vkCreateDebugUtilsMessengerEXT, &debugmessengerCreateInfo, nullptr, &debugHandle);
  }

  //-----------------------------------------------

  void destroy(VkInstance const &instance, VkDebugUtilsMessengerEXT &debugHandle)
  {
    if (!vo::sHasValidationLayers) return;
    vku__instanceFn(instance, vkDestroyDebugUtilsMessengerEXT, debugHandle, nullptr);
  }

  //-----------------------------------------------

}  // namespace debugmessenger

//=============================================================================

//

//=============================================================================
// ---- Queue Families ----
//=============================================================================

namespace queuefamily
{  //

  //-----------------------------------------------

  bool isComplete(std::vector<std::optional<uint32_t>> const indices)
  {
    for (auto &&i : indices) {
      if (!i.has_value()) { return false; }
    }
    return true;
  };

  //-----------------------------------------------

  std::vector<uint32_t> unrollOptionals(std::vector<std::optional<uint32_t>> const indices)
  {
    std::vector<uint32_t> indicesValue;
    indicesValue.reserve(indices.size());
    for (auto &&i : indices) { indicesValue.push_back(i.value()); }
    return indicesValue;
  };

  //-----------------------------------------------

  std::vector<uint32_t> getUniqueIndices(std::vector<uint32_t> const &indices)
  {
    std::set<uint32_t> uniqueIndices { indices.begin(), indices.end() };
    return std::vector<uint32_t> { uniqueIndices.begin(), uniqueIndices.end() };
  }

  //-----------------------------------------------

  std::vector<std::optional<uint32_t>> findIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
  {
    if (physicalDevice == VK_NULL_HANDLE or surface == VK_NULL_HANDLE) {
      vo__assert(0);
      return {};
    }

    // . Get device families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    // . Indices to find
    std::optional<uint32_t> graphicsIndex;
    std::optional<uint32_t> presentIndex;
    auto const              isComplete = [&]() { return graphicsIndex.has_value() && presentIndex.has_value(); };

    // . Find
    for (uint32_t i = 0u; (!isComplete() and i < queueFamilies.size()); ++i) {
      // .. Graphics
      if (queueFamilies.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) { graphicsIndex = i; }
      // .. Present
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
      if (presentSupport) { presentIndex = i; }
    }

    std::vector<std::optional<uint32_t>> indices(2);
    indices[Type::graphics] = graphicsIndex;
    indices[Type::present]  = presentIndex;
    return indices;
  }

  //-----------------------------------------------

  std::vector<VkQueue> findQueues(VkDevice device, std::vector<uint32_t> const &indices)
  {
    std::vector<VkQueue> queues(2);
    vkGetDeviceQueue(device, indices[Type::graphics], 0, &queues[Type::graphics]);
    vkGetDeviceQueue(device, indices[Type::present], 0, &queues[Type::present]);
    return queues;
  }

  //-----------------------------------------------

}  // namespace queuefamily

//=============================================================================

//

//=============================================================================
// --- SWAP CHAIN ---
//=============================================================================

namespace swapchain
{  //

  //---------------------------------------------

  std::tuple<VkSurfaceCapabilitiesKHR, std::vector<VkPresentModeKHR>, std::vector<VkSurfaceFormatKHR>> getSupportData(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR     surface)
  {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkPresentModeKHR>   presentModes;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;

    // . Capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
    // . Surface Formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount != 0) {
      surfaceFormats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());
    }
    // . Present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
      presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
    }

    return { capabilities, presentModes, surfaceFormats };
  }

  //---------------------------------------------

  bool isEmpty(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
  {
    auto const [capabilities, presentModes, surfaceFormats] = getSupportData(physicalDevice, surface);
    return presentModes.empty() or surfaceFormats.empty();
  }

  //---------------------------------------------

  Settings getSettings(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, Settings rs)
  {
    Settings scs;
    auto const [capabilities, presentModes, surfaceFormats] = getSupportData(physicalDevice, surface);

    // . [capabilities] - Direct store
    scs.capabilities = capabilities;

    // . [capabilities] - Get max allowed image to get in queue
    scs.minImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 and scs.minImageCount > capabilities.maxImageCount) {
      scs.minImageCount = capabilities.maxImageCount;
    }

    // . [capabilities] - Get best extent
    if (capabilities.currentExtent.width != UINT32_MAX) {
      scs.extent2D = capabilities.currentExtent;
    } else {
      uint32_t    w   = rs.extent2D.width;
      uint32_t    h   = rs.extent2D.height;
      auto const &min = capabilities.minImageExtent;
      auto const &max = capabilities.maxImageExtent;
      scs.extent2D    = VkExtent2D { std::clamp(w, min.width, max.width), std::clamp(h, min.height, max.height) };
    }

    // . [presentModes] - Get best
    // VK_PRESENT_MODE_MAILBOX_KHR : Render as fast as possible while still avoiding tearing : vsync + triple-buffer
    // VK_PRESENT_MODE_FIFO_KHR    : Guaranteed to be available : vsync + double-buffer
    scs.presentMode = VK_PRESENT_MODE_FIFO_KHR;  // fallback
    if (std::find(presentModes.begin(), presentModes.end(), rs.presentMode) != presentModes.end()) {
      scs.presentMode = rs.presentMode;
    }

    // . [surfaceFormat] - Get best
    for (const auto &available : surfaceFormats) {
      bool const sameFormat     = available.format == rs.surfaceFormat.format;
      bool const sameColorSpace = available.colorSpace == rs.surfaceFormat.colorSpace;
      if (sameFormat and sameColorSpace) { scs.surfaceFormat = available; }
    }

    return scs;
  }

  //---------------------------------------------

  void Settings::dumpInfo() const
  {
    vo__info("- SwapChainSettings -");
    vo__infof("Image Count             : {}", minImageCount);
    vo__infof("Selected Extent 2D      : ({},{})", extent2D.width, extent2D.height);
    vo__infof("Selected Present Mode   : {}", vku::ToStr_PresentMode.at(presentMode));
    vo__infof("Selected Surface Format : {}", vku::ToStr_Format.at(surfaceFormat.format));
  }

  //---------------------------------------------

}  // namespace swapchain

//=============================================================================

//

//=============================================================================
// --- SHADERS ---
//=============================================================================

namespace shaders
{  //

  //---------------------------------------------

  std::string getPath(char const *shaderName, VkShaderStageFlagBits stage)
  {
    static std::unordered_map<VkShaderStageFlagBits, std::string> stageToExtension {
      { VK_SHADER_STAGE_VERTEX_BIT, "vert" },
      { VK_SHADER_STAGE_FRAGMENT_BIT, "frag" },
      { VK_SHADER_STAGE_COMPUTE_BIT, "comp" },
      { VK_SHADER_STAGE_GEOMETRY_BIT, "geom" },
      { VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tesc" },
      { VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tese" },
    };

    static auto const VwShadersPath = std::string("./assets/shaders/");  // get this path from #define
    return VwShadersPath + shaderName + "." + stageToExtension[stage] + ".spv";
  };

  //---------------------------------------------

  ShaderData create(VkDevice logicalDevice, std::string const &name, VkShaderStageFlagBits stage)
  {
    auto const path = getPath(name.c_str(), stage);

    auto const code = vo::files::read(path);
    if (code.empty()) { vo__errf("Failed to open shader '{}'!", path); }

    VkShaderModuleCreateInfo const shadermoduleCreateInfo {
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = vku__castsize(code.size()),
      .pCode    = reinterpret_cast<const uint32_t *>(code.data()),
    };

    VkShaderModule shaderModule;
    vku__check(vkCreateShaderModule(logicalDevice, &shadermoduleCreateInfo, nullptr, &shaderModule));

    VkPipelineShaderStageCreateInfo shaderStageInfo {
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage  = stage,
      .module = shaderModule,
      .pName  = "main",  // Entrypoint function name
    };

    return { path, shaderModule, shaderStageInfo };
  }

  //---------------------------------------------

}  // namespace shaders

//=============================================================================

//

//=============================================================================
// ---  ---
//=============================================================================

//=============================================================================

//

//=============================================================================
// ---  ---
//=============================================================================

//=============================================================================

//

//=============================================================================
// ---  ---
//=============================================================================

}  // namespace vo::vulkan

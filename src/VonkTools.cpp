#include "VonkTools.h"
#include "VonkToStr.h"

#include "Utils.h"

namespace vonk
{  //

//

//=============================================================================
// ---- Memory ----
//=============================================================================

namespace memory
{  //

  /**
   * Get the index of a memory type that has all the requested property bits set
   * @param typeBits Bit mask with bits set for each memory type supported by the resource to request for (from
   * VkMemoryRequirements)
   * @param properties Bit mask of properties for the memory type to request.
   * @param (Optional) memTypeFound Pointer to a bool that is set to true if a matching memory type has been found.
   * @return Index of the requested memory type.
   */
  uint32_t getType(VkPhysicalDeviceMemoryProperties memProps, uint32_t typeBits, VkMemoryPropertyFlags requestedProps)
  {
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
      bool const propsMatch = (memProps.memoryTypes[i].propertyFlags & requestedProps) == requestedProps;
      if ((typeBits & 1) == 1 && propsMatch) { return i; }
      typeBits >>= 1;
    }
    vo__abort("Couldn't get requested memory properties");
    return 0;
  }

}  // namespace memory

//=============================================================================

//

//=============================================================================
// ---- Others ----
//=============================================================================

namespace others
{  //

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
        vonk::ToStr_DebugSeverity.at(messageSeverity),
        vonk::ToStr_DebugType.at(messageType),
        pUserData ? fmt::format("- {}", pUserData) : std::string { "" },
        pCallbackData->pMessage);

      CACHE[ID] = true;
    }

    return VK_FALSE;
  }

  //-----------------------------------------------

  void create(
    VkInstance const &               instance,
    VkDebugUtilsMessengerEXT &       debugHandle,
    std::vector<const char *> const &validationLayers)
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

    if (validationLayers.empty()) return;
    vonk__instanceFn(instance, vkCreateDebugUtilsMessengerEXT, &debugmessengerCreateInfo, nullptr, &debugHandle);
  }

  //-----------------------------------------------

  void destroy(
    VkInstance const &               instance,
    VkDebugUtilsMessengerEXT &       debugHandle,
    std::vector<const char *> const &validationLayers)
  {
    if (validationLayers.empty()) return;
    vonk__instanceFn(instance, vkDestroyDebugUtilsMessengerEXT, debugHandle, nullptr);
  }

  //-----------------------------------------------

}  // namespace debugmessenger

//=============================================================================

//

//=============================================================================
// ---- Queues ----
//=============================================================================

namespace queue
{  //

  //-----------------------------------------------

  bool isComplete(IndicesOpt const &indicesOpt)
  {
    return true                                 //
           and indicesOpt.graphics.has_value()  //
           and indicesOpt.present.has_value()   //
                                                //  and indicesOpt.compute.has_value()   //
                                                //  and indicesOpt.transfer.has_value()  //
      ;
  };

  //-----------------------------------------------

  QueueIndices_t unrollOptionals(IndicesOpt const &indicesOpt)
  {
    QueueIndices_t indices;
    indices.graphics = indicesOpt.graphics.value();
    indices.present  = indicesOpt.present.value();
    // indices.compute  = indicesOpt.compute.value();
    // indices.transfer  = indicesOpt.transfer.value();
    return indices;
  };

  //-----------------------------------------------

  std::vector<uint32_t> getUniqueIndices(QueueIndices_t const &indices)
  {
    std::set<uint32_t> uniqueIndices { indices.graphics, indices.present, /* indices.compute, indices.transfer */ };
    return std::vector<uint32_t> { uniqueIndices.begin(), uniqueIndices.end() };
  }

  //-----------------------------------------------

  IndicesOpt findIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
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
    IndicesOpt indicesOpt;

    // . Find
    for (uint32_t i = 0u; (!isComplete(indicesOpt) and i < queueFamilies.size()); ++i) {
      // .. Graphics
      if (queueFamilies.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) { indicesOpt.graphics = i; }
      // .. Present
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
      if (presentSupport) { indicesOpt.present = i; }
    }

    return indicesOpt;
  }

  //-----------------------------------------------

  Queues_t findQueues(VkDevice device, QueueIndices_t const &indices)
  {
    Queues_t queues;
    vkGetDeviceQueue(device, indices.graphics, 0, &queues.graphics);
    vkGetDeviceQueue(device, indices.present, 0, &queues.present);
    // vkGetDeviceQueue(device, indices.compute, 0, &queues.compute);
    // vkGetDeviceQueue(device, indices.transfer, 0, &queues.transfer);
    return queues;
  }

  //-----------------------------------------------

}  // namespace queue

//=============================================================================

//

//=============================================================================
// --- SWAP CHAIN ---
//=============================================================================

namespace swapchain
{  //

  //---------------------------------------------

  SupportSurface getSupportData(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
  {
    SupportSurface ssurf;

    // . Capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &ssurf.caps);

    // . Surface Formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount != 0) {
      ssurf.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, ssurf.formats.data());
    }

    // . Present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
      ssurf.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, ssurf.presentModes.data());
    }

    // . Depth format
    for (auto &format : { VK_FORMAT_D32_SFLOAT_S8_UINT,
                          VK_FORMAT_D32_SFLOAT,
                          VK_FORMAT_D24_UNORM_S8_UINT,
                          VK_FORMAT_D16_UNORM_S8_UINT,
                          VK_FORMAT_D16_UNORM }) {
      VkFormatProperties formatProps;
      vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
      // Format must support depth stencil attachment for optimal tiling
      if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        ssurf.depthFormat = format;
      }
    }

    return ssurf;
  }

  //---------------------------------------------

  bool isEmpty(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
  {
    auto const surfSupp = getSupportData(physicalDevice, surface);
    return surfSupp.presentModes.empty() or surfSupp.formats.empty();
  }

  //---------------------------------------------

  SwapShainSettings_t getSettings(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapShainSettings_t rs)
  {
    SwapShainSettings_t scs;
    scs.vsync        = rs.vsync;
    auto const ssurf = getSupportData(physicalDevice, surface);

    // . [capabilities] - Direct store
    // scs.capabilities = ssurf.caps;

    // . [capabilities] - Get max allowed image to get in queue
    scs.minImageCount = ssurf.caps.minImageCount + 1;
    if (ssurf.caps.maxImageCount > 0 and scs.minImageCount > ssurf.caps.maxImageCount) {
      scs.minImageCount = ssurf.caps.maxImageCount;
    }

    // . [capabilities] - Get best extent
    if (ssurf.caps.currentExtent.width != UINT32_MAX) {
      scs.extent2D = ssurf.caps.currentExtent;
    } else {
      uint32_t    w   = rs.extent2D.width;
      uint32_t    h   = rs.extent2D.height;
      auto const &min = ssurf.caps.minImageExtent;
      auto const &max = ssurf.caps.maxImageExtent;
      scs.extent2D    = VkExtent2D { std::clamp(w, min.width, max.width), std::clamp(h, min.height, max.height) };
    }

    // . [capabilities] - Get best transformation
    if (ssurf.caps.supportedTransforms & rs.preTransformFlag) {
      scs.preTransformFlag = rs.preTransformFlag;
    } else {
      scs.preTransformFlag = ssurf.caps.currentTransform;
    }

    // . [capabilities] - Get best alpha-composite
    if (ssurf.caps.supportedCompositeAlpha & rs.compositeAlphaFlag) {
      scs.compositeAlphaFlag = rs.compositeAlphaFlag;
    } else {
      static std::vector<VkCompositeAlphaFlagBitsKHR> const compositeAlphaFlags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
      };
      for (auto &compositeAlphaFlag : compositeAlphaFlags) {
        if (ssurf.caps.supportedCompositeAlpha & compositeAlphaFlag) {
          scs.compositeAlphaFlag = compositeAlphaFlag;
          break;
        };
      }
    }

    // . [capabilities] - Get image usage flags
    if (ssurf.caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
      scs.extraImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (ssurf.caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
      scs.extraImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    // . [presentModes] - Get best
    // * VK_PRESENT_MODE_FIFO_KHR    : Guaranteed to be available : vsync + double-buffer
    // * VK_PRESENT_MODE_MAILBOX_KHR :
    //    Render as fast as possible while still avoiding tearing : *almost* vsync + triple-buffer
    //    NOTE: Do not use it on mobiles due to power-consumption !!
    scs.presentMode = VK_PRESENT_MODE_FIFO_KHR;  // fallback
    if (!scs.vsync) {
      if (std::find(ssurf.presentModes.begin(), ssurf.presentModes.end(), rs.presentMode) != ssurf.presentModes.end()) {
        scs.presentMode = rs.presentMode;
      }
    }

    // . [surfaceFormat] - Get best
    bool isValidSurfaceFormat = false;
    for (const auto &available : ssurf.formats) {
      if (available.format == rs.colorFormat && available.colorSpace == rs.colorSpace) {
        isValidSurfaceFormat = true;
        break;
      }
    }
    if (isValidSurfaceFormat) {
      scs.colorFormat = rs.colorFormat;
      scs.colorSpace  = rs.colorSpace;
    } else {
      scs.colorFormat = ssurf.formats[0].format;
      scs.colorSpace  = ssurf.formats[0].colorSpace;
    }
    scs.depthFormat = ssurf.depthFormat;

    return scs;
  }

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
    static std::unordered_map<VkShaderStageFlagBits, std::string> sStageToExtension {
      { VK_SHADER_STAGE_VERTEX_BIT, "vert" },
      { VK_SHADER_STAGE_FRAGMENT_BIT, "frag" },
      { VK_SHADER_STAGE_COMPUTE_BIT, "comp" },
      { VK_SHADER_STAGE_GEOMETRY_BIT, "geom" },
      { VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tesc" },
      { VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tese" },
    };

    static auto const sShadersPath = std::string("./assets/shaders/");  // get this path from #define

    return sShadersPath + shaderName + "." + sStageToExtension[stage] + ".spv";
  };

    //---------------------------------------------

#define VONK_SHADER_CACHE 0

  ShaderData create(VkDevice device, std::string const &name, VkShaderStageFlagBits stage)
  {
    std::string const path = getPath(name.c_str(), stage);

    // . Evaluate cache first for early return if found
#if VONK_SHADER_CACHE
    //  NOTE: Temporary disabled due to requirement of ref_counting system to avoid destoy
    //  the shadermodule if other item is using it from the cache.
    static std::unordered_map<std::string, ShaderData> cache = {};
    if (cache.count(path) > 0) {
      vo__info("CACHED!!!");
      return cache.at(path);
    }
#endif

    // . Otherwise create the shader for first time
    std::vector<char> const code = vo::files::read(path);
    if (code.empty()) { vo__errf("Failed to open shader '{}'!", path); }

    VkShaderModuleCreateInfo const shadermoduleCI {
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = vonk__getSize(code),
      .pCode    = vonk__getDataAs(const uint32_t *, code),
    };
    VkShaderModule shadermodule;
    vonk__check(vkCreateShaderModule(device, &shadermoduleCI, nullptr, &shadermodule));

    VkPipelineShaderStageCreateInfo const shaderStageCI {
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage  = stage,
      .module = shadermodule,
      .pName  = "main",  // MUST : Entrypoint function name
    };

// . Finally store this on the cache-map and return
#if VONK_SHADER_CACHE
    cache[path] = { path, shadermodule, shaderStageCI };
    return cache.at(path);
#else
    return { path, shadermodule, shaderStageCI };
#endif
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

}  // namespace vonk

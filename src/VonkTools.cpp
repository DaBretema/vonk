#include "VonkTools.h"
#include "VonkToStr.h"

#include "Utils.h"

namespace vonk
{  //

//-----------------------------------------------

uint32_t
  getMemoryType(VkPhysicalDeviceMemoryProperties memProps, uint32_t typeBits, VkMemoryPropertyFlags requestedProps)
{
  for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
    bool const propsMatch = (memProps.memoryTypes[i].propertyFlags & requestedProps) == requestedProps;
    if ((typeBits & 1) == 1 && propsMatch) { return i; }
    typeBits >>= 1;
  }
  Abort("Couldn't get requested memory properties");
  return 0;
}

//-----------------------------------------------

SurfaceSupport_t getSurfaceSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
  SurfaceSupport_t SS;

  // . Capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &SS.caps);

  // . Surface Formats
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    SS.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, SS.formats.data());
  }

  // . Present modes
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    SS.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, SS.presentModes.data());
  }

  // . Depth format
  for (auto &format : { VK_FORMAT_D32_SFLOAT_S8_UINT,
                        VK_FORMAT_D32_SFLOAT,
                        VK_FORMAT_D24_UNORM_S8_UINT,
                        VK_FORMAT_D16_UNORM_S8_UINT,
                        VK_FORMAT_D16_UNORM }) {
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(gpu, format, &formatProps);
    // Format must support depth stencil attachment for optimal tiling
    if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) { SS.depthFormat = format; }
  }

  return SS;
}

//-----------------------------------------------

bool checkGpuExtensionsSupport(Gpu_t const &gpu)
{
  if (gpu.exts.empty()) return true;

  uint32_t count;
  vkEnumerateDeviceExtensionProperties(gpu.handle, nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> available(count);
  vkEnumerateDeviceExtensionProperties(gpu.handle, nullptr, &count, available.data());

  std::set<std::string> required(gpu.exts.begin(), gpu.exts.end());
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

}  // namespace vonk

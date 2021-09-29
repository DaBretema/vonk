#include "VwSwapChain.h"
#include "VwToStr.h"

namespace vo
{
//=============================================================================

VkExtent2D clampToImageExtent(uint32_t w, uint32_t h, VkSurfaceCapabilitiesKHR const &capabilities)
{
  auto const &min = capabilities.minImageExtent;
  auto const &max = capabilities.maxImageExtent;
  return { std::clamp(w, min.width, max.width), std::clamp(h, min.height, max.height) };
}
//=============================================================================

void SwapChainSettings::dumpInfo() const
{
  VO_INFO("- SwapChainSettings -");
  VO_INFO_FMT("Image Count             : {}", minImageCount);
  VO_INFO_FMT("Selected Extent 2D      : ({},{})", extent2D.width, extent2D.height);
  VO_INFO_FMT("Selected Present Mode   : {}", VwToStr_PresentMode.at(presentMode));
  VO_INFO_FMT("Selected Surface Format : {}", VwToStr_Format.at(surfaceFormat.format));
}

//=============================================================================

namespace swapchain
{
  //-----------------------------------------------

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

  //-----------------------------------------------

  bool isEmpty(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
  {
    auto const [capabilities, presentModes, surfaceFormats] = getSupportData(physicalDevice, surface);
    return presentModes.empty() or surfaceFormats.empty();
  }

  //-----------------------------------------------

  SwapChainSettings getSettings(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapChainRequestedSettings rs)
  {
    SwapChainSettings scs;
    auto const [capabilities, presentModes, surfaceFormats] = getSupportData(physicalDevice, surface);

    // . [capabilities] - Direct store
    scs.capabilities = capabilities;

    // . [capabilities] - Get max allowed image to get in queue
    scs.minImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 and scs.minImageCount > capabilities.maxImageCount) {
      scs.minImageCount = capabilities.maxImageCount;
    }

    // . [capabilities] - Get best extent
    auto const currExtent        = capabilities.currentExtent;
    bool const currExtentIsValid = currExtent.width != UINT32_MAX;
    scs.extent2D = (currExtentIsValid) ? currExtent : clampToImageExtent(rs.windowW, rs.windowH, capabilities);

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
}  // namespace swapchain

//=============================================================================

}  // namespace vo

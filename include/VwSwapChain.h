#pragma once
#include "Macros.h"
#include "VwVulkan.h"

namespace vo
{
//=============================================================================

struct SwapChainRequestedSettings
{
  uint32_t           windowW       = 0;
  uint32_t           windowH       = 0;
  VkPresentModeKHR   presentMode   = VK_PRESENT_MODE_MAILBOX_KHR;
  VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
};

//=============================================================================

struct SwapChainSettings
{
  uint32_t           minImageCount = 0u;
  VkExtent2D         extent2D      = { 1280, 720 };
  VkPresentModeKHR   presentMode   = VK_PRESENT_MODE_FIFO_KHR;
  VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

  VkSurfaceCapabilitiesKHR capabilities;

  void dumpInfo() const;
};

//=============================================================================

namespace swapchain
{
  bool              isEmpty(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
  SwapChainSettings getSettings(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapChainRequestedSettings rs);
}  // namespace swapchain

//=============================================================================

}  // namespace vo

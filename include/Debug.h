#pragma once

#include "_vulkan.h"
#include "Macros.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT             messageType,
  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
  void *                                      pUserData)
{
  VO_ERR_FMT("Validation layer issue: {}", pCallbackData->pMessage);

  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    // Message is important enough to show
  }

  return VK_FALSE;
}

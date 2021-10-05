#include "VwDebugMessenger.h"

#include "Macros.h"
#include "Settings.h"
#include "VwToStr.h"

namespace vku
{
static inline VKAPI_ATTR VkBool32 VKAPI_CALL sDebugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT             messageType,
  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
  MBU void *                                  pUserData)
{
  static std::unordered_map<int32_t, bool> cache {};
  auto const                               ID = pCallbackData->messageIdNumber;

  if (!cache[ID] && messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    fmt::print(
      "\n ❗️ [VALIDATION LAYERS] - {} at {}{}"
      "\n--------------------------------------------------------------------------------\n{}\n",
      vku::ToStr_DebugSeverity.at(messageSeverity),
      vku::ToStr_DebugType.at(messageType),
      pUserData ? fmt::format("- {}", pUserData) : std::string { "" },
      pCallbackData->pMessage);

    cache[ID] = true;
  }

  return VK_FALSE;
}

//=============================================================================

DebugMessenger::DebugMessenger()
{
  createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                           | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = sDebugCallback;
  createInfo.pUserData       = nullptr;  // Optional
  createInfo.pNext           = nullptr;  // Mandatory
  createInfo.flags           = 0;        // Mandatory
}

//-----------------------------------------------

void DebugMessenger::create(VkInstance const &instance)
{
  if (!vo::sHasValidationLayers) return;

  VW_INSTANCE_FN(instance, vkCreateDebugUtilsMessengerEXT, &createInfo, nullptr, &debugMessenger);
}

//-----------------------------------------------

void DebugMessenger::destroy(VkInstance const &instance)
{
  if (!vo::sHasValidationLayers) return;

  VW_INSTANCE_FN(instance, vkDestroyDebugUtilsMessengerEXT, debugMessenger, nullptr);
}

//-----------------------------------------------
}  // namespace vku

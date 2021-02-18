#include <stdint.h>
#include <array>
#include <functional>

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include <fmt/core.h>

namespace Vonsai::vk0
{
//===============================================
// FUNCTIONS
//===============================================

// Debug callback
//-----------------------------------------------------------------------------

VkResult CreateDebugUtilsMessengerEXT(
  VkInstance                                instance,
  const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
  const VkAllocationCallbacks *             pAllocator,
  VkDebugUtilsMessengerEXT *                pCallback);

//-----------------------------------------------------------------------------

void DestroyDebugUtilsMessengerEXT(
  VkInstance                   instance,
  VkDebugUtilsMessengerEXT     callback,
  const VkAllocationCallbacks *pAllocator);

//-----------------------------------------------------------------------------

vk::DebugUtilsMessengerCreateInfoEXT defaultDebugUtilsMessengerCreateInfo();

//-----------------------------------------------------------------------------

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT             messageType,
  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
  void *                                      pUserData);

//-----------------------------------------------------------------------------

//===============================================
// CONSTANTS
//===============================================

// Validation Layers
//-----------------------------------------------------------------------------

const std::array LAYERS { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
constexpr bool LAYERS_ON = false;
#else
constexpr bool LAYERS_ON = true;
#endif

//-----------------------------------------------------------------------------

//===============================================
// MACROS
//===============================================

// Pretty print  ( https://stackoverflow.com/a/3048361 )
//-----------------------------------------------------------------------------

#define VK0_DefaultErrorMsg    "[ERR] - Vulkan - {}:{}\n↪ {}\n"
#define VK0_Alert(msg)         fmt::print(VK0_DefaultErrorMsg, __FILE__, __LINE__, msg)
#define VK0_AlertFmt(msg, ...) fmt::print(VK0_DefaultErrorMsg, __FILE__, __LINE__, fmt::format(msg, __VA_ARGS__))

//-----------------------------------------------------------------------------

// Error capture wrapper
//-----------------------------------------------------------------------------

#define VK0_Test(res, msg)           \
  if (res != vk::Result::eSuccess) { \
    VK0_Alert(msg);                  \
    abort();                         \
  }
#define VK0_Test_C(res, msg) \
  if (res != VK_SUCCESS) {   \
    VK0_Alert(msg);          \
    abort();                 \
  }
#define VK0_Test_Fn(fn)             \
  if (fn != vk::Result::eSuccess) { \
    VK0_Alert(#fn);                 \
    abort();                        \
  }
#define VK0_Test_Fn_C(fn) \
  if (fn != VK_SUCCESS) { \
    VK0_Alert(#fn);       \
    abort();              \
  }

#define VK0_Call(fn)          \
  [&]() -> auto               \
  {                           \
    auto [res, val] = vk::fn; \
    VK0_Test(res, #fn);       \
    return std::move(val);    \
  }                           \
  ()

//-----------------------------------------------------------------------------

// C++ to C container size and data
//-----------------------------------------------------------------------------

#define VK0_SizeData(container) static_cast<uint32_t>(container.size()), container.data()

//-----------------------------------------------------------------------------

//===============================================
//===============================================
}  // namespace Vonsai::vk0

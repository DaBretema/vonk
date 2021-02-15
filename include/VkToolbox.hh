#include <stdint.h>
#include <array>

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

namespace Vonsai::vk0
{
//===============================================
// CONSTANTS
//===============================================

#ifdef NDEBUG
constexpr bool LAYERS_ON = false;
#else
constexpr bool LAYERS_ON = true;
#endif
const std::array LAYERS { "VK_LAYER_KHRONOS_validation" };

//===============================================
// MACROS
//===============================================

#define vk0Say(msg) fmt::print("[VK - ERR] ({}:{}) :: '{}'\n", __FILE__, __LINE__, msg)
#define vk0Err(r, msg)             \
  if (r == vk::Result::eSuccess) { \
    vk0Say(msg);                   \
    abort();                       \
  }

}  // namespace Vonsai::vk0

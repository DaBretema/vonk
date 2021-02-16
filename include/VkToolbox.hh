#include <stdint.h>
#include <array>
#include <functional>

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

namespace Vonsai::vk0
{
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

#define VK0_DEF_ERR             "[ERR] - Vulkan - {}:{}\n↪ {}\n"
#define VK0_Alert(msg)          fmt::print(VK0_DEF_ERR, __FILE__, __LINE__, msg)
#define VK0_Alert_Fmt(msg, ...) fmt::print(VK0_DEF_ERR, __FILE__, __LINE__, fmt::format(msg, __VA_ARGS__))

//-----------------------------------------------------------------------------

// Error capture wrapper
//-----------------------------------------------------------------------------

#define VK0_Call(fn)                   \
  [&]() -> auto                        \
  {                                    \
    auto [res, val] = vk::fn;          \
                                       \
    if (res == vk::Result::eSuccess) { \
      VK0_Alert(#fn);                  \
      abort();                         \
    }                                  \
                                       \
    return std::move(val);             \
  }                                    \
  ()

//-----------------------------------------------------------------------------

// C++ to C container size and data
//-----------------------------------------------------------------------------

#define VK0_SizeData(container) static_cast<uint32_t>(container.size()), container.data()

//-----------------------------------------------------------------------------

//===============================================
//===============================================
}  // namespace Vonsai::vk0

#pragma once

#include <array>

namespace vk0
{

//-----------------------------------------------------------------------------
// === EXTENSIONS
//-----------------------------------------------------------------------------

#if 1
inline constexpr std::array<const char *, 0> extensions {};
// inline constexpr std::array<const char *, 0> deviceExtensions {};
#else
inline constexpr std::array extensions { "VK_KHR_portability_subset" };
// constexpr std::array extensions { "VK_KHR_get_physical_device_properties2", "VK_KHR_portability_subset" };
#endif

[[maybe_unused]] inline constexpr std::array deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//-----------------------------------------------------------------------------
// === VALIDATION LAYERS
//-----------------------------------------------------------------------------

#if NDEBUG
inline constexpr std::array<const char *, 0> LAYERS {};
#else
inline constexpr std::array layers { "VK_LAYER_KHRONOS_validation" };
// constexpr std::array LAYERS { "VK_LAYER_LUNARG_standard_validation" };
#endif
inline constexpr bool hasLayers { !layers.empty() };

//-----------------------------------------------------------------------------
// ===
//-----------------------------------------------------------------------------

}  // namespace vk0

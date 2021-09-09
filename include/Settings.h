#pragma once

#include <array>

namespace vo
{

// === EXTENSIONS
//-----------------------------------------------------------------------------

#define EXTENSION_SET 0

#    if EXTENSION_SET==0
inline constexpr std::array<const char *, 0> sExtensions {};
#elif EXTENSION_SET==1
//inline constexpr std::array sExtensions { "VK_KHR_portability_subset" };
#elif EXTENSION_SET==2
//inline constexpr std::array sExtensions { "VK_KHR_get_physical_device_properties2", "VK_KHR_portability_subset" };
#endif


// === DEVICE EXTENSIONS
//-----------------------------------------------------------------------------

#define DEVICE_EXTENSION_SET 0

#    if DEVICE_EXTENSION_SET==0
[[maybe_unused]] inline constexpr std::array<std::string,0> sDeviceExtensions{};
#elif DEVICE_EXTENSION_SET==1
[[maybe_unused]] inline constexpr std::array sDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
#endif


// === VALIDATION LAYERS
//-----------------------------------------------------------------------------

#define VALIDATION_SET 0

#    if VALIDATION_SET==0
inline constexpr std::array<const char *, 0> sLayers {};
#elif VALIDATION_SET==1
inline constexpr std::array sLayers { "VK_LAYER_KHRONOS_validation" };
#endif

inline constexpr bool sHasLayers { !sLayers.empty() };


}  // namespace vk0

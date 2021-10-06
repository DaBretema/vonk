#pragma once
#include <array>
#include <VwVulkan.h>

namespace vo
{
// === FOR SHADERS
//-----------------------------------------------------------------------------
#define VO_HARDCODED_SHAPE 1  // 0:triangle, 1:quad, ...

// === OTHER DEFINES
//-----------------------------------------------------------------------------
#define VO_VERBOSE            0
#define VO_VERBOSE_EXTENSIONS 0
#define VO_FUNCTION_LINE_LOG  1

// === INSTANCE EXTENSIONS
//-----------------------------------------------------------------------------

#define INSTANCE_EXTENSION_SET 1

#if INSTANCE_EXTENSION_SET == 0
inline constexpr std::array<const char *, 0> sInstanceExtensions {};
#elif INSTANCE_EXTENSION_SET == 1
inline constexpr std::array                  sInstanceExtensions { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
#elif INSTANCE_EXTENSION_SET == 2
inline constexpr std::array                  sInstanceExtensions { VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                                  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
#endif

inline constexpr bool sHasInstanceExtensions { !sInstanceExtensions.empty() };

// === DEVICE EXTENSIONS
//-----------------------------------------------------------------------------

#define DEVICE_EXTENSION_SET 2

#if DEVICE_EXTENSION_SET == 0
[[maybe_unused]] inline constexpr std::array<const char *, 0> sDeviceExtensions {};
#elif DEVICE_EXTENSION_SET == 1
[[maybe_unused]] inline constexpr std::array sDeviceExtensions = { "VK_KHR_portability_subset" };
#elif DEVICE_EXTENSION_SET == 2
[[maybe_unused]] inline constexpr std::array sDeviceExtensions = { "VK_KHR_portability_subset",
                                                                   VK_KHR_SWAPCHAIN_EXTENSION_NAME };
#endif

inline constexpr bool sHasDeviceExtensions { !sDeviceExtensions.empty() };

// === VALIDATION LAYERS
//-----------------------------------------------------------------------------

#define VALIDATION_SET 1

#if VALIDATION_SET == 0
inline constexpr std::array<const char *, 0> sValidationLayers {};
#elif VALIDATION_SET == 1
inline constexpr std::array                  sValidationLayers { "VK_LAYER_KHRONOS_validation" };
#endif

inline constexpr bool sHasValidationLayers { !sValidationLayers.empty() };

//-----------------------------------------------------------------------------

}  // namespace vo

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

#define INSTANCE_EXTENSION_SET 0

#if INSTANCE_EXTENSION_SET == 0
inline const std::vector<const char *> sInstanceExtensions {};
#elif INSTANCE_EXTENSION_SET == 1
inline const std::vector<const char *> sInstanceExtensions { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
#endif

inline const bool sHasInstanceExtensions { !sInstanceExtensions.empty() };

// === DEVICE EXTENSIONS
//-----------------------------------------------------------------------------

#define DEVICE_EXTENSION_SET 1

#if DEVICE_EXTENSION_SET == 0
inline const std::vector<const char *> sDeviceExtensions {};
#elif DEVICE_EXTENSION_SET == 1
inline const std::vector<const char *> sDeviceExtensions = { "VK_KHR_portability_subset",
                                                             VK_KHR_SWAPCHAIN_EXTENSION_NAME };
#endif

inline const bool sHasDeviceExtensions { !sDeviceExtensions.empty() };

// === VALIDATION LAYERS
//-----------------------------------------------------------------------------

#define VALIDATION_SET 1

#if VALIDATION_SET == 0
inline const std::vector<const char *> sValidationLayers {};
#elif VALIDATION_SET == 1
inline const std::vector<const char *> sValidationLayers { "VK_LAYER_KHRONOS_validation" };
#endif

inline const bool sHasValidationLayers { !sValidationLayers.empty() };

//-----------------------------------------------------------------------------

}  // namespace vo

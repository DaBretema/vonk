#include <stdint.h>
#include <array>

namespace Vonsai::vk0
{
//===============================================

#ifdef NDEBUG
constexpr bool LAYERS_ON = false;
#else
constexpr bool LAYERS_ON = true;
#endif
const std::array LAYERS { "VK_LAYER_KHRONOS_validation" };

//===============================================

#define vkErr_(msg) fmt::print("[VK - ERR] ({}:{}) :: '{}'", __FILE__, __LINE__, msg)
#define vkErr(r, msg) \
  if (r != vk::Result::eSuccess) vkErr_(msg)

//===============================================
}  // namespace Vonsai::vk0

#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

namespace vo
{
//-----------------------------------------------------------------------------
// === PRINTERS
//-----------------------------------------------------------------------------
#ifndef NDEBUG
#  define VO_TRACE(lvl, msg) fmt::print("{} {}\n", std::string(lvl, '>'), msg)
#else
#  define VO_TRACE(s)
#endif

#define VO_ERR(msg)          fmt::print("[VO_ERR] - {}:{}\n↪ {}\n", __FILE__, __LINE__, msg)
#define VO_ERR_FMT(msg, ...) VO_ERR(fmt::format(msg, __VA_ARGS__))

#define VO_ABORT(msg) \
  VO_ERR(msg);        \
  abort();

#define VO_ABORT_FMT(msg) \
  VO_ERR_FMT(msg);        \
  abort();

#define VO_CHECK(conditionCode) \
  if (!conditionCode) { VO_ABORT(#conditionCode); }

//-----------------------------------------------------------------------------
// === VULKAN MACROS
//-----------------------------------------------------------------------------

#define VX_CHECK(vulkanCode) \
  if (vulkanCode != VK_SUCCESS) { VO_ABORT(#vulkanCode); }

#define VX_SIZE_CAST(v) static_cast<uint32_t>(v)

//-----------------------------------------------------------------------------
// === Get EXT functions
//-----------------------------------------------------------------------------

#define vk0InstanceFn(instance, extName, ...) \
  if (auto fn = ((PFN_##extName)vkGetInstanceProcAddr(instance, #extName)); fn) { fn(instance, __VA_ARGS__); }

#define vk0InstanceResFn(instance, extName, ...)                                  \
  if (auto fn = ((PFN_##extName)vkGetInstanceProcAddr(instance, #extName)); fn) { \
    vk0TestFnC(fn(instance, __VA_ARGS__));                                        \
  }

}  // namespace vo

#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

#include "Settings.h"

namespace vo
{
//

// ::: Mark argunment as unused to avoid warnings
#define MBU [[maybe_unused]]

// === PRINTERS
//-----------------------------------------------------------------------------
#ifndef NDEBUG
#  define VO_TRACE(lvl, msg) fmt::print("{} {}\n", std::string(lvl, '>'), msg)
#  if VO_FUNCTION_LINE_LOG
#    define VO_INFO(msg) fmt::print("ℹ️ ({}:{}) → {}\n", __FILE__, __LINE__, msg)
#    define VO_ERR(msg)  fmt::print("⚠️ ({}:{}) → {}\n", __FILE__, __LINE__, msg)
// #    define VO_ERR(msg)  fmt::print("[VO_ERR] ({}:{})\n↪ {}\n", __FILE__, __LINE__, msg)
#  else
#    define VO_INFO(msg) fmt::print("ℹ️  {}\n", msg)
#    define VO_ERR(msg)  fmt::print("⚠️  {}\n", msg)
#  endif
#else
#  define VO_TRACE(s)
#endif

#define VO_INFO_FMT(msg, ...) VO_INFO(fmt::format(msg, __VA_ARGS__))
#define VO_ERR_FMT(msg, ...)  VO_ERR(fmt::format(msg, __VA_ARGS__))

#define VO_ABORT(msg) \
  VO_ERR(msg);        \
  abort();

#define VO_ABORT_FMT(msg, ...)  \
  VO_ERR_FMT(msg, __VA_ARGS__); \
  abort();

#define VO_CHECK(conditionCode) \
  if (!conditionCode) { VO_ABORT(#conditionCode); }

//-----------------------------------------------------------------------------
// === VULKAN MACROS = VW_
//-----------------------------------------------------------------------------

// ::: Validate api calls
#define VW_CHECK(vulkanCode) \
  if (VkResult res = vulkanCode; res != VK_SUCCESS) { VO_ABORT_FMT("{} : {}", res, #vulkanCode); }

// ::: Cast shortcut
#define VW_SIZE_CAST(v) static_cast<uint32_t>(v)

// ::: Get instance functions
#define VW_INSTANCE_FN(instance, extName, ...)                                    \
  if (auto fn = ((PFN_##extName)vkGetInstanceProcAddr(instance, #extName)); fn) { \
    fn(instance, __VA_ARGS__);                                                    \
  } else {                                                                        \
    VO_ERR_FMT("Function {} is not available", #extName);                         \
  }

}  // namespace vo

#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

#include "Settings.h"

namespace vo
{
//

// ::: Mark argunment as unused to avoid warnings
#define MBU [[maybe_unused]]
static inline auto const PtrStr = [](auto *p) { return fmt::format("{}", fmt::ptr(p)); };

//
// === PRINTERS
//-----------------------------------------------------------------------------
#ifndef NDEBUG
static inline auto const VO_TRACE = [](int lvl, auto msg) {
  std::string const beginLineJumps = lvl < 2 ? "\n\n" : lvl < 3 ? "\n" : "";

  std::string const byLevelSep  = std::string(4 * (lvl - 1), ' ');
  std::string const byLevelMark = lvl < 2 ? "### " : lvl < 3 ? ">> " : lvl < 4 ? "* " : "- ";
  // std::string const byLevelMark = std::string(lvl, '>') + ' ';

  size_t            textSize       = std::string(msg).size() + byLevelMark.size();
  std::string const byLevelSubLine = lvl < 3 ? std::string(textSize, '-') + "\n" : "";

  std::string const message = byLevelSep + byLevelMark + msg;
  std::string const subLine = "\n" + (lvl < 3 ? byLevelSep : "") + byLevelSubLine;

  fmt::print("{}{}{}", beginLineJumps, message, subLine);
};
#  if VO_FUNCTION_LINE_LOG
#    define VO_INFO(msg) fmt::print("ℹ️  ({}:{}) → {}\n", __FILE__, __LINE__, msg)
#    define VO_ERR(msg)  fmt::print("⚠️  ({}:{}) → {}\n", __FILE__, __LINE__, msg)
// #    define VO_ERR(msg)  fmt::print("[VO_ERR] ({}:{})\n↪ {}\n", __FILE__, __LINE__, msg)
#  else
#    define VO_INFO(msg) fmt::print("ℹ️  {}\n", msg)
#    define VO_ERR(msg)  fmt::print("⚠️  {}\n", msg)
#  endif
#else
#  define VO_INFO(msg) fmt::print("ℹ️  {}\n", msg)
#  define VO_ERR(msg)  fmt::print("⚠️  {}\n", msg)
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

// //
// // === LOGIC ABSTRACTION : PATHS
// //-----------------------------------------------------------------------------

// auto const VO__INTERNAL__GET_SHADER_PATH = [](char const *shaderName, char const *extension) {
//   static auto const VwShadersPath = std::string("./assets/shaders/");
//   return VwShadersPath + shaderName + "." + extension + ".spv";
// };
// #define VO_GET_SHADER_PATH_VERT(shaderName) VO__INTERNAL__GET_SHADER_PATH(shaderName, "vert")
// #define VO_GET_SHADER_PATH_FRAG(shaderName) VO__INTERNAL__GET_SHADER_PATH(shaderName, "frag")
// #define VO_GET_SHADER_PATH_COMP(shaderName) VO__INTERNAL__GET_SHADER_PATH(shaderName, "comp")

//
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

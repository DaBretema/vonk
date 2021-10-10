#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

#include "Settings.h"

namespace vo
{  //

// ::: Mark argunment as unused to avoid warnings
#define MBU [[maybe_unused]]
static inline auto const PtrStr = [](auto *p) { return fmt::format("{}", fmt::ptr(p)); };

//.
//.
//.

//
// === VONSAI MACROS = vo__
//-----------------------------------------------------------------------------
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

// "Raw" log macros
#define vo__info(msg) fmt::print("ℹ️  ({}:{}) → {}\n", __FILE__, __LINE__, msg)
#define vo__warn(msg) fmt::print("⚠️  ({}:{}) → {}\n", __FILE__, __LINE__, msg)
#define vo__err(msg)  fmt::print("⛔️  ({}:{}) → {}\n", __FILE__, __LINE__, msg)
#define vo__abort(msg) \
  vo__err(msg);        \
  abort();

// "Formatable" log macros
#define vo__infof(msg, ...) vo__info(fmt::format(msg, __VA_ARGS__))
#define vo__warnf(msg, ...) vo__err(fmt::format(msg, __VA_ARGS__))
#define vo__errf(msg, ...)  vo__err(fmt::format(msg, __VA_ARGS__))
#define vo__abortf(msg, ...)  \
  vo__errf(msg, __VA_ARGS__); \
  abort();

// Evaluate critical condition
#define vo__check(conditionCode) \
  if (!(conditionCode)) { vo__abort(#conditionCode); }

// Custom Assert
#define vo__assert(cond) assert(cond)

//.
//.
//.

// === VULKAN MACROS = vku__
//-----------------------------------------------------------------------------

// ::: Validate api calls
#define vku__check(vulkanCode) \
  if (VkResult res = vulkanCode; res != VK_SUCCESS) { vo__abortf("{} : {}", res, #vulkanCode); }

// ::: Cast shortcut
#define vku__castsize(v) static_cast<uint32_t>(v)

// ::: Get instance functions
#define vku__instanceFn(instance, extName, ...)                                   \
  if (auto fn = ((PFN_##extName)vkGetInstanceProcAddr(instance, #extName)); fn) { \
    fn(instance, __VA_ARGS__);                                                    \
  } else {                                                                        \
    vo__errf("Function {} is not available", #extName);                           \
  }

}  // namespace vo

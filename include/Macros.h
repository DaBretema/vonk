#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

//-----------------------------------------------

//
// // === C++ Helpers
//

//-----------------------------------------------

#define MBU [[maybe_unused]]

//-----------------------------------------------

//
// // === VONSAI MACROS = vo__
//

//-----------------------------------------------

// Logging w/o format
#define vo__info(msg) fmt::print("ℹ️  ({}:{}) → {}\n", __FILE__, __LINE__, msg)
#define vo__warn(msg) fmt::print("⚠️  ({}:{}) → {}\n", __FILE__, __LINE__, msg)
#define vo__err(msg)  fmt::print("⛔️  ({}:{}) → {}\n", __FILE__, __LINE__, msg)
#define vo__abort(msg) \
  vo__err(msg);        \
  abort();

//-----------------------------------------------

// Logging w/ format
#define vo__infof(msg, ...) vo__info(fmt::format(msg, __VA_ARGS__))
#define vo__warnf(msg, ...) vo__err(fmt::format(msg, __VA_ARGS__))
#define vo__errf(msg, ...)  vo__err(fmt::format(msg, __VA_ARGS__))
#define vo__abortf(msg, ...)  \
  vo__errf(msg, __VA_ARGS__); \
  abort();

//-----------------------------------------------

// Evaluate critical condition
#define vo__check(conditionCode) \
  if (!(conditionCode)) { vo__abort(#conditionCode); }

//-----------------------------------------------

// Custom Assert
#define vo__assert(cond) assert(cond)

//-----------------------------------------------

// Fmt ptr
#define vo__ptrstr(p) fmt::format("{}", fmt::ptr(p))

//-----------------------------------------------

//
// // === VULKAN MACROS = vonk__
//

//-----------------------------------------------

// Validate api calls
#define vonk__check(vulkanCode) \
  if (VkResult res = vulkanCode; res != VK_SUCCESS) { vo__abortf("{} : {}", res, #vulkanCode); }

//-----------------------------------------------

// Vector C++ to C helpers
#define vonk__castSize(v)           static_cast<uint32_t>(v)
#define vonk__getSize(v)            vonk__castSize(v.size())
#define vonk__getData(v)            v.data()
#define vonk__getDataAs(newType, v) reinterpret_cast<newType>(vonk__getData(v))

//-----------------------------------------------

// Get instance functions
#define vonk__instanceFn(instance, extName, ...)                                  \
  if (auto fn = ((PFN_##extName)vkGetInstanceProcAddr(instance, #extName)); fn) { \
    fn(instance, __VA_ARGS__);                                                    \
  } else {                                                                        \
    vo__errf("Function {} is not available", #extName);                           \
  }

//-----------------------------------------------

#pragma once

// --- Platform dependencies

// #define VO_USE_OTHER_PLATFORMS
//==========
#ifdef VO_USE_OTHER_PLATFORMS
// More info : https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/WSIheaders.html
//==========
#elif __APPLE__
#  define GLFW_EXPOSE_NATIVE_COCOA
#  include "TargetConditionals.h"
#  if TARGET_OS_OSX == 1
#    define VK_USE_PLATFORM_MACOS_MVK
#  elif TARGET_OS_IPHONE == 1 || TARGET_OS_SIMULATOR == 1
#    define VK_USE_PLATFORM_IOS_MVK
#  endif
//==========
#elif _WIN64 || _WIN32
#  define GLFW_EXPOSE_NATIVE_WIN32
#  define VK_USE_PLATFORM_WIN32_KHR
//==========
#elif __linux || __unix || __posix
// TODO: Get specific platform based on server
//-----
#  define GLFW_EXPOSE_NATIVE_X11
#  define VK_USE_PLATFORM_XLIB_KHR
// #define VK_USE_PLATFORM_XCB_KHR
//-----
// #  define GLFW_EXPOSE_NATIVE_WAYLAND
// #define VK_USE_PLATFORM_WAYLAND_KHR
//-----
// #define VK_USE_PLATFORM_MIR_KHR
// #define VK_USE_PLATFORM_XLIB_XRANDR_EXT
//-----
//==========
#elif __ANDROID__
#  define VK_USE_PLATFORM_ANDROID_KHR
#endif
//-----------------------------------------------

//
// === Vulkan spececifics

//-----------------------------------------------

#define VK_ENABLE_BETA_EXTENSIONS

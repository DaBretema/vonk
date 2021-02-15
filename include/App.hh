#pragma once

#include <GLFW/glfw3.h>
#include <algorithm>
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

namespace Vonsai
{
//---------------------------------------------------------------------------

template<typename T, typename V>
bool isOn(T container, V val)
{
  return std::find(begin(container), end(container), val) != container.end();
}

//---------------------------------------------------------------------------

class App
{
public:
  void run();

private:
  vk::UniqueInstance mInstance;
  GLFWwindow *       mWindow { nullptr };

  void initWindow();
  void initVulkan();
  void mainLoop();
  void cleanup();

  void createInstance();

  bool checkValidationLayerSupport();
};

}  // namespace Vonsai

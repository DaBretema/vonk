#include "Vonsai.h"

namespace vo
{
//-----------------------------------------------

void Vonsai::run()
{
  //===== Window

  vo::window::init(1280, 720, "Vonsai");

  //===== Vulkan Create
  mVulkan.init();

  // // ..  Create instance
  // mInstance.create();
  // vo__check(mInstance.handle);

  // // ..  Create device
  // mDevice.create(mInstance);
  // vo__check(mDevice.handle);

  // // ..  Populate shaders
  // static auto const createShadersSameName =
  //   [&](std::string const &name, std::initializer_list<VkShaderStageFlagBits> stages) {
  //     for (auto const stage : stages) {
  //       auto const data = vku::shaders::create(mDevice.handle, name, stage);
  //       mDevice.shaderModules.emplace(data.path, data.module);
  //       mDevice.pipelineShaderStageCreateInfos.emplace_back(data.stageCreateInfo);
  //     }
  //   };
  // createShadersSameName("base", { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT });

  // // ..  GraphicsPipeline
  // mDevice.createGraphicsPipeline();

  //===== Loop

  vo::window::loop(
    [&]() { mVulkan.drawFrame(); },   //
    [&]() { mVulkan.waitDevice(); },  //
    false                             //
  );

  //===== Clean up
  mVulkan.cleanup();
  // mDevice.destroy();
  // mInstance.destroy();

  //=====
}

//-----------------------------------------------

//.............................................................................
//.............................................................................
//.............................................................................

//-----------------------------------------------

//.............................................................................
//.............................................................................
//.............................................................................

}  // namespace vo

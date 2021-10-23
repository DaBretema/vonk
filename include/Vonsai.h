#pragma once

#include <string>
#include <vector>

#include "VoWindow.h"
#include "VoVulkanBase.h"

namespace vo
{  //

class Vonsai
{
public:
  Vonsai(uint32_t w, uint32_t h, std::string title) : mTitle(std::move(title)), mW(w), mH(h) {}
  void run();

private:
  vku::Base   mVulkan = {};
  std::string mTitle  = { "Vonsai!" };
  uint32_t    mW = { 1280 }, mH = { 720 };
};

}  // namespace vo

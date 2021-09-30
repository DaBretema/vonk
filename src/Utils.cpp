#include "Utils.h"
#include "Macros.h"

#include <fstream>

namespace vo::files
{
//

//-----------------------------------------------

// ::: Read file content
std::vector<char> read(std::string const &filepath)
{
  // . Open file from the end or return empty
  auto file = std::ifstream { filepath, std::ios::ate | std::ios::binary };
  if (!file.is_open()) { return {}; }

  // . Create a buffer with the size of the file
  size_t            fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  // . Move back to file begins, read and close
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

//-----------------------------------------------

}  // namespace vo::files

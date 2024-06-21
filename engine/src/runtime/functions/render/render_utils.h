#pragma once
#include <fstream>
#include <string>
#include <vector>

namespace peanut {
class RenderUtils {
 public:
  template <typename T>
  static constexpr bool IsPowerOfTwo(T value) {
    return value != 0 && (value & (value - 1)) == 0;
  }

  template <typename T>
  static constexpr T RoundToPowerOfTwo(T value, int POT) {
    return (value + POT - 1) & -POT;
  }

  template <typename T>
  static constexpr T NumMipmapLevels(T width, T height) {
    T levels = 1;
    while ((width | height) >> levels) {
      ++levels;
    }
    return levels;
  }

  template <typename T>
  static bool CheckDoubleNearZero(T value) {
    PEANUT_LOG_ERROR("Only support double and float");
  }

  template <>
  static bool CheckDoubleNearZero<double>(double value) {
    return std::abs(value - 0.0f) <= std::numeric_limits<double>::epsilon();
  }

  template <>
  static bool CheckDoubleNearZero<float>(float value) {
    return std::abs(value - 0.0f) <= std::numeric_limits<float>::epsilon();
  }

    static bool ReadBinaryFile(const std::string& path,
                                std::vector<char>& output) 
    {
        std::ifstream file{path, std::ios::binary | std::ios::ate};
        if (!file.is_open())
        {
            return false;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        output.resize(size);
        file.read(output.data(), size);
        return true;
    }
};
}  // namespace peanut

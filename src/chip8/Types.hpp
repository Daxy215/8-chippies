#pragma once

#include <array>
#include <cstdint>

namespace chip8 {
    constexpr int DisplayWidth = 64;
    constexpr int DisplayHeight = 32;

    using PixelBuffer = std::array<std::uint32_t, DisplayWidth * DisplayHeight>;
    using KeyState = std::array<bool, 16>;
} // namespace chip8

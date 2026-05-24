#pragma once

#include <cstdint>

namespace chip8 {
    class Timers {
    public:
        void reset();

        void tick();

        [[nodiscard]] std::uint8_t delay() const;

        [[nodiscard]] std::uint8_t sound() const;

        void setDelay(std::uint8_t value);

        void setSound(std::uint8_t value);

    private:
        std::uint8_t delayTimer = 0;
        std::uint8_t soundTimer = 0;
    };
} // namespace chip8

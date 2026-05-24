#include "Timer.hpp"

namespace chip8 {
    void Timers::reset() {
        delayTimer = 0;
        soundTimer = 0;
    }

    void Timers::tick() {
        if (delayTimer > 0) {
            delayTimer--;
        }

        if (soundTimer > 0) {
            soundTimer--;
        }
    }

    std::uint8_t Timers::delay() const {
        return delayTimer;
    }

    std::uint8_t Timers::sound() const {
        return soundTimer;
    }

    void Timers::setDelay(std::uint8_t value) {
        delayTimer = value;
    }

    void Timers::setSound(std::uint8_t value) {
        soundTimer = value;
    }
} // namespace chip8

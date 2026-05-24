#include "Interconnect.hpp"

#include "Gpu.hpp"

namespace chip8 {
    void Interconnect::reset() {
        cpu.reset();
        gpu::clearScreen(framebuffer);
        timerAccumulator = 0.0;
    }

    void Interconnect::stepOnce() {
        cpu.step(framebuffer, keys);
        cpu.tickTimers();
    }

    void Interconnect::runFrame(int cyclesPerFrame, double elapsedSeconds) {
        for (int cycle = 0; cycle < cyclesPerFrame; cycle++) {
            cpu.step(framebuffer, keys);
        }

        timerAccumulator += elapsedSeconds;
        while (timerAccumulator >= TimerStepSeconds) {
            cpu.tickTimers();
            timerAccumulator -= TimerStepSeconds;
        }
    }

    void Interconnect::setKeys(const KeyState &newKeys) {
        keys = newKeys;
    }

    bool Interconnect::beeping() const {
        return cpu.soundTimer() > 0;
    }
} // namespace chip8

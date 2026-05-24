#pragma once

#include "Cpu.hpp"
#include "Types.hpp"

namespace chip8 {
    class Interconnect {
    public:
        Cpu cpu;
        PixelBuffer framebuffer{};
        KeyState keys{};

        void reset();

        void stepOnce();

        void runFrame(int cyclesPerFrame, double elapsedSeconds);

        void setKeys(const KeyState &newKeys);

        [[nodiscard]] bool beeping() const;

    private:
        double timerAccumulator = 0.0;
        static constexpr double TimerStepSeconds = 1.0 / 60.0;
    };
} // namespace chip8

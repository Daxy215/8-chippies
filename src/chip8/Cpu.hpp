#pragma once

#include "Timer.hpp"
#include "Types.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <random>
#include <string>

namespace chip8 {
    class Cpu {
    public:
        bool loadRom(const std::filesystem::path &path, std::string &error);

        void reset();

        void step(PixelBuffer &framebuffer, const KeyState &keys);

        void tickTimers();

        [[nodiscard]] std::uint16_t pc() const;

        [[nodiscard]] std::uint16_t index() const;

        [[nodiscard]] std::uint16_t stackPointer() const;

        [[nodiscard]] std::size_t romSize() const;

        [[nodiscard]] std::uint8_t delayTimer() const;

        [[nodiscard]] std::uint8_t soundTimer() const;

    private:
        static constexpr std::size_t StackSize = 16;
        static constexpr std::size_t MemorySize = 4096;
        static constexpr std::uint16_t FontStart = 0x050;
        static constexpr std::uint16_t FontBytesPerCharacter = 5;
        static constexpr std::uint16_t ProgramStart = 0x200;

        void pushStack(std::uint16_t address);

        std::uint16_t popStack();

        void loadFontSet();

        [[nodiscard]] static bool isKeyPressed(std::uint8_t key, const KeyState &keys);

        [[nodiscard]] static int firstPressedKey(const KeyState &keys);

        std::uint16_t PC = ProgramStart;
        std::uint16_t I = 0;
        std::uint16_t SP = 0;

        std::size_t loadedRomSize = 0;
        std::array<std::uint8_t, MemorySize> ram{};
        std::array<std::uint16_t, StackSize> stack{};
        std::array<std::uint8_t, 16> V{};
        Timers timers;

        std::mt19937 randomGenerator{std::random_device{}()};
        std::uniform_int_distribution<int> randomByte{0, 255};

        bool shiftUsesVy = false;
    };
} // namespace chip8

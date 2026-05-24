#include "Cpu.hpp"

#include "Gpu.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <vector>

namespace chip8 {
    namespace {
        constexpr std::array<std::uint8_t, 80> FontSet{
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80 // F
        };
    }

    bool Cpu::loadRom(const std::filesystem::path &path, std::string &error) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);

        if (!file) {
            error = "Could not open ROM: " + path.string();
            return false;
        }

        const std::streamsize fileSize = file.tellg();
        if (fileSize <= 0) {
            error = "ROM is empty: " + path.string();
            return false;
        }

        const auto maxRomSize = static_cast<std::streamsize>(MemorySize - ProgramStart);
        if (fileSize > maxRomSize) {
            error = "ROM is too large for CHIP-8 memory.";
            return false;
        }

        std::vector<std::uint8_t> rom(static_cast<std::size_t>(fileSize));
        file.seekg(0, std::ios::beg);

        if (!file.read(reinterpret_cast<char *>(rom.data()), fileSize)) {
            error = "Failed to read ROM: " + path.string();
            return false;
        }

        reset();
        std::ranges::copy(rom, ram.begin() + ProgramStart);
        loadedRomSize = rom.size();
        PC = ProgramStart;
        error.clear();

        return true;
    }

    void Cpu::reset() {
        ram.fill(0);
        loadFontSet();
        PC = ProgramStart;
        I = 0;
        SP = 0;
        loadedRomSize = 0;
        stack.fill(0);
        V.fill(0);
        timers.reset();
    }

    void Cpu::tickTimers() {
        timers.tick();
    }

    void Cpu::step(PixelBuffer &framebuffer, const KeyState &keys) {
        const auto instruction = static_cast<std::uint16_t>(ram[PC] << 8 | ram[PC + 1]);
        PC += 2;

        const std::uint16_t opcode = instruction;
        const std::uint8_t op = (opcode & 0xF000) >> 12;
        const std::uint8_t x = (opcode & 0x0F00) >> 8;
        const std::uint8_t y = (opcode & 0x00F0) >> 4;
        const std::uint8_t n = opcode & 0x000F;
        const std::uint8_t nn = opcode & 0x00FF;
        const std::uint16_t nnn = opcode & 0x0FFF;

        // Decode by the first nibble, then use X/Y/N/NN/NNN as operands.
        switch (op) {
            case 0x0: {
                switch (opcode) {
                    case 0x0000:
                        break;
                    case 0x00E0:
                        // 00E0: Clear the display.
                        gpu::clearScreen(framebuffer);
                        break;
                    case 0x00EE:
                        // 00EE: Return from subroutine.
                        PC = popStack();
                        break;
                    default:
                        // 0NNN: SYS addr. Ignored by modern CHIP-8 interpreters.
                        break;
                }
                break;
            }
            case 0x1:
                // 1NNN: Jump to NNN.
                PC = nnn;
                break;
            case 0x2:
                // 2NNN: Call subroutine at NNN.
                pushStack(PC);
                PC = nnn;
                break;
            case 0x3:
                // 3XNN: Skip next instruction if VX == NN.
                if (V[x] == nn) {
                    PC += 2;
                }
                break;
            case 0x4:
                // 4XNN: Skip next instruction if VX != NN.
                if (V[x] != nn) {
                    PC += 2;
                }
                break;
            case 0x5:
                // 5XY0: Skip next instruction if VX == VY.
                if (n == 0 && V[x] == V[y]) {
                    PC += 2;
                }
                break;
            case 0x6:
                // 6XNN: Set VX = NN.
                V[x] = nn;
                break;
            case 0x7:
                // 7XNN: Add NN to VX. VF is not affected.
                V[x] += nn;
                break;
            case 0x8: {
                // 8XYN: Register and arithmetic instructions.
                switch (n) {
                    case 0x0:
                        // 8XY0: Set VX = VY.
                        V[x] = V[y];
                        break;
                    case 0x1:
                        // 8XY1: Set VX = VX OR VY.
                        V[x] |= V[y];
                        break;
                    case 0x2:
                        // 8XY2: Set VX = VX AND VY.
                        V[x] &= V[y];
                        break;
                    case 0x3:
                        // 8XY3: Set VX = VX XOR VY.
                        V[x] ^= V[y];
                        break;
                    case 0x4: {
                        // 8XY4: Add VY to VX. VF is set on carry.
                        const std::uint16_t value = V[x] + V[y];
                        V[0xF] = value > 0xFF ? 1 : 0;
                        V[x] = value & 0xFF;
                        break;
                    }
                    case 0x5:
                        // 8XY5: Subtract VY from VX. VF is 1 when no borrow occurs.
                        V[0xF] = V[x] >= V[y] ? 1 : 0;
                        V[x] -= V[y];
                        break;
                    case 0x6:
                        // 8XY6: Shift VX right. VF gets the shifted-out low bit.
                        if (shiftUsesVy) {
                            V[x] = V[y];
                        }
                        V[0xF] = V[x] & 0x1;
                        V[x] >>= 1;
                        break;
                    case 0x7: {
                        // 8XY7: Set VX = VY - VX. VF is 1 when no borrow occurs.
                        const std::uint8_t vx = V[x];
                        V[0xF] = V[y] >= vx ? 1 : 0;
                        V[x] = V[y] - vx;
                        break;
                    }
                    case 0xE:
                        // 8XYE: Shift VX left. VF gets the shifted-out high bit.
                        if (shiftUsesVy) {
                            V[x] = V[y];
                        }
                        V[0xF] = (V[x] & 0x80) >> 7;
                        V[x] <<= 1;
                        break;
                    default:
                        std::printf("Unknown 8XYN opcode: 0x%04X\n", opcode);
                        break;
                }
                break;
            }
            case 0x9:
                // 9XY0: Skip next instruction if VX != VY.
                if (n == 0 && V[x] != V[y]) {
                    PC += 2;
                }
                break;
            case 0xA:
                // ANNN: Set index register I = NNN.
                I = nnn;
                break;
            case 0xB:
                // BNNN: Jump to NNN + V0.
                PC = nnn + V[0];
                break;
            case 0xC:
                // CXNN: Set VX = random byte AND NN.
                V[x] = static_cast<std::uint8_t>(randomByte(randomGenerator) & nn);
                break;
            case 0xD: {
                // DXYN: Draw N-byte sprite from memory at I to screen at VX, VY.
                // VF is set if drawing turns any already-on pixel off.
                const std::uint8_t vx = V[x] % DisplayWidth;
                const std::uint8_t vy = V[y] % DisplayHeight;
                V[0xF] = 0;

                for (int row = 0; row < n; row++) {
                    const std::uint8_t pixelState = ram[I + row];

                    for (int bit = 0; bit < 8; bit++) {
                        const bool spritePixelOn = (pixelState & (0x80 >> bit)) != 0;
                        const int pixelX = (vx + bit) % DisplayWidth;
                        const int pixelY = (vy + row) % DisplayHeight;

                        if (!spritePixelOn) {
                            continue;
                        }

                        if (gpu::isPixelOn(pixelX, pixelY, framebuffer)) {
                            gpu::setPixel(pixelX, pixelY, false, framebuffer);
                            V[0xF] = 1;
                        } else {
                            gpu::setPixel(pixelX, pixelY, true, framebuffer);
                        }
                    }
                }
                break;
            }
            case 0xE: {
                switch (nn) {
                    case 0x9E:
                        // EX9E: Skip next instruction if key in VX is pressed.
                        if (isKeyPressed(V[x], keys)) {
                            PC += 2;
                        }
                        break;
                    case 0xA1:
                        // EXA1: Skip next instruction if key in VX is not pressed.
                        if (!isKeyPressed(V[x], keys)) {
                            PC += 2;
                        }
                        break;
                    default:
                        std::printf("Unknown EXNN opcode: 0x%04X\n", opcode);
                        break;
                }
                break;
            }
            case 0xF: {
                switch (nn) {
                    case 0x07:
                        // FX07: Set VX to delay timer value.
                        V[x] = timers.delay();
                        break;
                    case 0x0A: {
                        // FX0A: Wait for a key press, then store the key in VX.
                        const int pressedKey = firstPressedKey(keys);
                        if (pressedKey < 0) {
                            PC -= 2;
                            break;
                        }

                        V[x] = static_cast<std::uint8_t>(pressedKey);
                        break;
                    }
                    case 0x15:
                        // FX15: Set delay timer = VX.
                        timers.setDelay(V[x]);
                        break;
                    case 0x18:
                        // FX18: Set sound timer = VX.
                        timers.setSound(V[x]);
                        break;
                    case 0x1E:
                        // FX1E: Add VX to index register I.
                        I += V[x];
                        break;
                    case 0x29:
                        // FX29: Set I to the font sprite address for the hex digit in VX.
                        I = FontStart + (V[x] & 0x0F) * FontBytesPerCharacter;
                        break;
                    case 0x33: {
                        // FX33: Store binary-coded decimal digits of VX at I, I+1, I+2.
                        const std::uint8_t value = V[x];
                        ram[I] = value / 100;
                        ram[I + 1] = (value / 10) % 10;
                        ram[I + 2] = value % 10;
                        break;
                    }
                    case 0x55:
                        // FX55: Store V0 through VX into memory starting at I.
                        for (int reg = 0; reg <= x; reg++) {
                            ram[I + reg] = V[reg];
                        }
                        break;
                    case 0x65:
                        // FX65: Load V0 through VX from memory starting at I.
                        for (int reg = 0; reg <= x; reg++) {
                            V[reg] = ram[I + reg];
                        }
                        break;
                    default:
                        std::printf("Unknown FXNN opcode: 0x%04X\n", opcode);
                        break;
                }
                break;
            }
            default:
                std::printf("Unknown opcode: 0x%04X\n", opcode);
                break;
        }
    }

    std::uint16_t Cpu::pc() const {
        return PC;
    }

    std::uint16_t Cpu::index() const {
        return I;
    }

    std::uint16_t Cpu::stackPointer() const {
        return SP;
    }

    std::size_t Cpu::romSize() const {
        return loadedRomSize;
    }

    std::uint8_t Cpu::delayTimer() const {
        return timers.delay();
    }

    std::uint8_t Cpu::soundTimer() const {
        return timers.sound();
    }

    void Cpu::pushStack(std::uint16_t address) {
        if (SP >= stack.size()) {
            std::fprintf(stderr, "Stack overflow while pushing address 0x%03X\n", address);
            return;
        }

        stack[SP++] = address;
    }

    std::uint16_t Cpu::popStack() {
        if (SP == 0) {
            std::fprintf(stderr, "Stack underflow while returning from subroutine\n");
            return PC;
        }

        return stack[--SP];
    }

    void Cpu::loadFontSet() {
        std::ranges::copy(FontSet, ram.begin() + FontStart);
    }

    bool Cpu::isKeyPressed(std::uint8_t key, const KeyState &keys) {
        return key < keys.size() && keys[key];
    }

    int Cpu::firstPressedKey(const KeyState &keys) {
        for (std::size_t key = 0; key < keys.size(); key++) {
            if (keys[key]) {
                return static_cast<int>(key);
            }
        }

        return -1;
    }
} // namespace chip8

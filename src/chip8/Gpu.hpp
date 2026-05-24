#pragma once

#include "Types.hpp"

#include <GLFW/glfw3.h>

namespace chip8::gpu {
    void clearScreen(PixelBuffer &framebuffer);

    void setPixel(int x, int y, bool on, PixelBuffer &framebuffer);

    bool isPixelOn(int x, int y, const PixelBuffer &framebuffer);

    class Texture {
    public:
        Texture();

        ~Texture();

        Texture(const Texture &) = delete;

        Texture &operator=(const Texture &) = delete;

        Texture(Texture &&other) noexcept;

        Texture &operator=(Texture &&other) noexcept;

        void upload(const PixelBuffer &framebuffer) const;

        [[nodiscard]] GLuint id() const;

    private:
        GLuint texture = 0;
    };
} // namespace chip8::gpu

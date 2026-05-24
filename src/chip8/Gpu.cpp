#include "Gpu.hpp"

#include <cstddef>
#include <utility>

namespace chip8::gpu {
    namespace {
        constexpr std::uint32_t OffColor = 0xff000000;
        constexpr std::uint32_t OnColor = 0xffffffff;
    }

    void clearScreen(PixelBuffer &framebuffer) {
        framebuffer.fill(OffColor);
    }

    void setPixel(int x, int y, bool on, PixelBuffer &framebuffer) {
        x %= DisplayWidth;
        y %= DisplayHeight;

        framebuffer[static_cast<std::size_t>(y * DisplayWidth + x)] = on ? OnColor : OffColor;
    }

    bool isPixelOn(int x, int y, const PixelBuffer &framebuffer) {
        x %= DisplayWidth;
        y %= DisplayHeight;

        return framebuffer[static_cast<std::size_t>(y * DisplayWidth + x)] == OnColor;
    }

    Texture::Texture() {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, DisplayWidth, DisplayHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    Texture::~Texture() {
        if (texture != 0) {
            glDeleteTextures(1, &texture);
        }
    }

    Texture::Texture(Texture &&other) noexcept
        : texture(std::exchange(other.texture, 0)) {
    }

    Texture &Texture::operator=(Texture &&other) noexcept {
        if (this == &other) {
            return *this;
        }

        if (texture != 0) {
            glDeleteTextures(1, &texture);
        }

        texture = std::exchange(other.texture, 0);
        return *this;
    }

    void Texture::upload(const PixelBuffer &framebuffer) const {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            DisplayWidth,
            DisplayHeight,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            framebuffer.data()
        );
    }

    GLuint Texture::id() const {
        return texture;
    }
} // namespace chip8::gpu

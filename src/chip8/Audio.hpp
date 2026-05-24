#pragma once

#include <atomic>

#include <miniaudio.h>

namespace chip8 {
    class AudioEngine {
    public:
        bool start();

        void stop();

        void setBeeping(bool value);

        void setVolume(float value);

        ~AudioEngine();

    private:
        static void audioCallback(ma_device *device, void *output, const void *input, ma_uint32 frameCount);

        ma_device device{};
        bool initialized = false;
        float sampleRate = 48000.0f;
        float phase = 0.0f;
        std::atomic<bool> beeping = false;
        std::atomic<float> volume = 0.25f;
    };
} // namespace chip8

#define MINIAUDIO_IMPLEMENTATION
#include "Audio.hpp"

#include <cmath>

namespace chip8 {
    namespace {
        constexpr float AudioFrequencyHz = 440.0f;
        constexpr float Pi = 3.14159265358979323846f;
    }

    bool AudioEngine::start() {
        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.format = ma_format_f32;
        config.playback.channels = 1;
        config.sampleRate = 48000;
        config.dataCallback = audioCallback;
        config.pUserData = this;

        if (ma_device_init(nullptr, &config, &device) != MA_SUCCESS) {
            return false;
        }

        sampleRate = static_cast<float>(device.sampleRate);
        if (ma_device_start(&device) != MA_SUCCESS) {
            ma_device_uninit(&device);
            initialized = false;
            return false;
        }

        initialized = true;
        return true;
    }

    void AudioEngine::stop() {
        if (!initialized) {
            return;
        }

        ma_device_uninit(&device);
        initialized = false;
    }

    void AudioEngine::setBeeping(bool value) {
        beeping.store(value, std::memory_order_relaxed);
    }

    void AudioEngine::setVolume(float value) {
        volume.store(value, std::memory_order_relaxed);
    }

    AudioEngine::~AudioEngine() {
        stop();
    }

    void AudioEngine::audioCallback(ma_device *device, void *output, const void *, ma_uint32 frameCount) {
        auto *audio = static_cast<AudioEngine *>(device->pUserData);
        auto *samples = static_cast<float *>(output);

        const bool shouldBeep = audio->beeping.load(std::memory_order_relaxed);
        const float currentVolume = audio->volume.load(std::memory_order_relaxed);
        const float phaseStep = (2.0f * Pi * AudioFrequencyHz) / audio->sampleRate;

        for (ma_uint32 frame = 0; frame < frameCount; frame++) {
            if (shouldBeep && currentVolume > 0.0f) {
                samples[frame] = std::sin(audio->phase) * currentVolume;
                audio->phase += phaseStep;

                if (audio->phase >= 2.0f * Pi) {
                    audio->phase -= 2.0f * Pi;
                }
            } else {
                samples[frame] = 0.0f;
            }
        }
    }
} // namespace chip8

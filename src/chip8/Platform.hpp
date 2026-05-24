#pragma once

#include <filesystem>
#include <string>

namespace chip8::platform {
    std::filesystem::path configFilePath();

    void saveLastRomPath(const std::string &path);

    std::string loadLastRomPath();

    std::string openRomFileDialog();
} // namespace chip8::platform

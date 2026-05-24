#include "Platform.hpp"

#include <array>
#include <cstdlib>
#include <cstdio>
#include <fstream>

#ifdef _WIN32
#define NOMINMAX
#include <commdlg.h>
#include <windows.h>
#endif

namespace chip8::platform {
    std::filesystem::path configFilePath() {
#ifdef _WIN32
        const char *appData = std::getenv("APPDATA");
        const std::filesystem::path base = appData != nullptr ? appData : ".";
        return base / "8-chippy" / "last_rom.txt";
#else
        if (const char *xdgConfigHome = std::getenv("XDG_CONFIG_HOME")) {
            return std::filesystem::path(xdgConfigHome) / "8-chippy" / "last_rom.txt";
        }

        if (const char *home = std::getenv("HOME")) {
            return std::filesystem::path(home) / ".config" / "8-chippy" / "last_rom.txt";
        }

        return std::filesystem::path(".8-chippy-last-rom");
#endif
    }

    void saveLastRomPath(const std::string &path) {
        const std::filesystem::path configPath = configFilePath();
        const std::filesystem::path parentPath = configPath.parent_path();

        if (!parentPath.empty()) {
            std::error_code error;
            std::filesystem::create_directories(parentPath, error);
        }

        std::ofstream file(configPath);
        if (file) {
            file << path << '\n';
        }
    }

    std::string loadLastRomPath() {
        std::ifstream file(configFilePath());
        if (!file) {
            return {};
        }

        std::string path;
        std::getline(file, path);
        return path;
    }

#ifdef _WIN32
    std::string openRomFileDialog() {
        char fileName[MAX_PATH] = {};

        OPENFILENAMEA dialog{};
        dialog.lStructSize = sizeof(dialog);
        dialog.lpstrFile = fileName;
        dialog.nMaxFile = sizeof(fileName);
        dialog.lpstrFilter = "CHIP-8 ROMs\0*.ch8;*.rom;*.bin\0All files\0*.*\0";
        dialog.nFilterIndex = 1;
        dialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        dialog.lpstrTitle = "Load CHIP-8 ROM";

        if (!GetOpenFileNameA(&dialog)) {
            return {};
        }

        return fileName;
    }
#else
    namespace {
        std::string trimTrailingNewline(std::string value) {
            while (!value.empty() && (value.back() == '\n' || value.back() == '\r')) {
                value.pop_back();
            }

            return value;
        }

        std::string readCommandOutput(const char *command) {
            FILE *pipe = popen(command, "r");
            if (pipe == nullptr) {
                return {};
            }

            std::array<char, 512> buffer{};
            std::string output;
            while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
                output += buffer.data();
            }

            pclose(pipe);
            return trimTrailingNewline(output);
        }
    } // namespace

    std::string openRomFileDialog() {
        const char *commands[] = {
            "zenity --file-selection --title='Load CHIP-8 ROM' --filename='./ROMS/' "
            "--file-filter='CHIP-8 ROMs | *.ch8 *.rom *.bin' --file-filter='All files | *' 2>/dev/null",
            "kdialog --getopenfilename ./ROMS 'CHIP-8 ROMs (*.ch8 *.rom *.bin)' 2>/dev/null",
            "yad --file-selection --title='Load CHIP-8 ROM' --filename='./ROMS/' "
            "--file-filter='CHIP-8 ROMs | *.ch8 *.rom *.bin' --file-filter='All files | *' 2>/dev/null"
        };

        for (const char *command: commands) {
            std::string selectedPath = readCommandOutput(command);
            if (!selectedPath.empty()) {
                return selectedPath;
            }
        }

        return {};
    }
#endif
} // namespace chip8::platform

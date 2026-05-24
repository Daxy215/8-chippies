#include "chip8/Audio.hpp"
#include "chip8/Gpu.hpp"
#include "chip8/Input.hpp"
#include "chip8/Interconnect.hpp"
#include "chip8/Platform.hpp"
#include "chip8/Types.hpp"

#include <cstdio>
#include <string>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace {
constexpr int WindowWidth = 1280;
constexpr int WindowHeight = 720;
constexpr const char* GlSlVersion = "#version 330";

struct AppState {
    bool running = false;
    int scale = 12;
    int cyclesPerFrame = 8;
    float volume = 0.25f;
    float clearColor[4] = {0.08f, 0.09f, 0.10f, 1.0f};
    char romPath[512] = "../ROMS/test_opcode.ch8";
    std::string status = "No ROM loaded.";
};

void glfwErrorCallback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

void copyRomPath(AppState& app, const std::string& path) {
    std::snprintf(app.romPath, sizeof(app.romPath), "%s", path.c_str());
}

void loadLastRomPath(AppState& app) {
    const std::string path = chip8::platform::loadLastRomPath();
    if (!path.empty()) {
        copyRomPath(app, path);
        app.status = "Restored previous ROM path.";
    }
}

void loadRomFromPath(chip8::Interconnect& bus, AppState& app) {
    std::string error;

    if (bus.cpu.loadRom(app.romPath, error)) {
        chip8::gpu::clearScreen(bus.framebuffer);
        app.running = false;
        chip8::platform::saveLastRomPath(app.romPath);
        app.status = "Loaded " + std::string(app.romPath) + " (" + std::to_string(bus.cpu.romSize()) + " bytes).";
        return;
    }

    app.running = false;
    app.status = error;
}

void browseAndLoadRom(chip8::Interconnect& bus, AppState& app) {
    const std::string selectedPath = chip8::platform::openRomFileDialog();
    if (selectedPath.empty()) {
        app.status = "No ROM selected. On Fedora, install zenity, kdialog, or yad for the file picker.";
        return;
    }

    copyRomPath(app, selectedPath);
    loadRomFromPath(bus, app);
}

void drawMenuBar(GLFWwindow* window, chip8::Interconnect& bus, AppState& app) {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Load ROM...")) {
            browseAndLoadRom(bus, app);
        }

        if (ImGui::MenuItem("Quit")) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Debug")) {
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void drawEmulatorPanel(chip8::Interconnect& bus, AppState& app, const chip8::gpu::Texture& displayTexture) {
    ImGui::Begin("CHIP-8");

    ImGui::InputText("ROM path", app.romPath, sizeof(app.romPath));

    if (ImGui::Button("Browse...")) {
        browseAndLoadRom(bus, app);
    }

    ImGui::SameLine();
    if (ImGui::Button("Load Path")) {
        loadRomFromPath(bus, app);
    }

    ImGui::SameLine();
    if (ImGui::Button(app.running ? "Pause" : "Run")) {
        app.running = !app.running;
    }

    ImGui::SameLine();
    if (ImGui::Button("Step")) {
        bus.stepOnce();
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        bus.reset();
        app.running = false;
        app.status = "CPU reset.";
    }

    ImGui::TextUnformatted(app.status.c_str());
    ImGui::SliderInt("Display scale", &app.scale, 4, 20);
    ImGui::SliderInt("Emulator speed", &app.cyclesPerFrame, 1, 60, "%d cycles/frame");
    ImGui::SliderFloat("Volume", &app.volume, 0.0f, 1.0f, "%.2f");

    const ImVec2 displaySize{
        static_cast<float>(chip8::DisplayWidth * app.scale),
        static_cast<float>(chip8::DisplayHeight * app.scale)
    };

    ImGui::SeparatorText("Display");
    ImGui::Image(
        static_cast<ImTextureID>(displayTexture.id()),
        displaySize,
        ImVec2(0.0f, 0.0f),
        ImVec2(1.0f, 1.0f)
    );

    ImGui::End();
}

void drawDebugPanel(const chip8::Interconnect& bus) {
    ImGui::Begin("Debug");
    ImGui::TextUnformatted("CPU state placeholder");

    ImGui::Separator();
    ImGui::Text("PC: 0x%03X", bus.cpu.pc());
    ImGui::Text("I:  0x%03X", bus.cpu.index());
    ImGui::Text("SP: 0x%02X", bus.cpu.stackPointer());
    ImGui::Text("DT: 0x%02X", bus.cpu.delayTimer());
    ImGui::Text("ST: 0x%02X", bus.cpu.soundTimer());
    ImGui::TextUnformatted(bus.beeping() ? "BEEP" : "");
    ImGui::Text("ROM: %zu bytes", bus.cpu.romSize());
    ImGui::End();
}
} // namespace

int main() {
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, "8 Chippy", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GlSlVersion);

    AppState app;
    chip8::Interconnect bus;
    loadLastRomPath(app);
    chip8::gpu::clearScreen(bus.framebuffer);

    chip8::AudioEngine audio;
    if (!audio.start()) {
        app.status = "Audio failed to start.";
    }

    chip8::gpu::Texture displayTexture;
    double previousTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        bus.setKeys(chip8::input::readKeys(window));

        const double currentTime = glfwGetTime();
        const double elapsedTime = currentTime - previousTime;
        previousTime = currentTime;

        if (app.running) {
            bus.runFrame(app.cyclesPerFrame, elapsedTime);
        }

        audio.setVolume(app.volume);
        audio.setBeeping(bus.beeping());

        displayTexture.upload(bus.framebuffer);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawMenuBar(window, bus, app);
        drawEmulatorPanel(bus, app, displayTexture);
        drawDebugPanel(bus);

        ImGui::Render();

        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

        glViewport(0, 0, framebufferWidth, framebufferHeight);
        glClearColor(app.clearColor[0], app.clearColor[1], app.clearColor[2], app.clearColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    audio.stop();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

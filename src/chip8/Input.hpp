#pragma once

#include "Types.hpp"

#include <GLFW/glfw3.h>

namespace chip8::input {
    KeyState readKeys(GLFWwindow *window);
} // namespace chip8::input

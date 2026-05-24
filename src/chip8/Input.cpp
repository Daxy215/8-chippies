#include "Input.hpp"

namespace chip8::input {
    KeyState readKeys(GLFWwindow *window) {
        KeyState keys{};

        keys[0x1] = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
        keys[0x2] = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
        keys[0x3] = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
        keys[0xC] = glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS;

        keys[0x4] = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
        keys[0x5] = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        keys[0x6] = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
        keys[0xD] = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;

        keys[0x7] = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        keys[0x8] = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        keys[0x9] = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
        keys[0xE] = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;

        keys[0xA] = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;
        keys[0x0] = glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS;
        keys[0xB] = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
        keys[0xF] = glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS;

        return keys;
    }
} // namespace chip8::input

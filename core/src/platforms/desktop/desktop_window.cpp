#include "desktop_window.hpp"

#include <iostream>

namespace aardvark {

const int STENCIL_BITS = 8;
const int MSAA_SAMPLE_COUNT = 4;  // 4;

DesktopWindow::DesktopWindow(DesktopApp* app, const Size& size)
    : app(app), size(size) {
    if (!glfwInit()) {
        std::cout << "glfw init error" << std::endl;
    }
    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, STENCIL_BITS);
    glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLE_COUNT);
    window = glfwCreateWindow(size.width, size.height, "GLFW", NULL, NULL);
    glfwSetWindowUserPointer(window, static_cast<void*>(this));
    make_current();
    glfwSwapInterval(0);
};

DesktopWindow::~DesktopWindow() { glfwDestroyWindow(window); };
void DesktopWindow::swap() { glfwSwapBuffers(window); };
void DesktopWindow::swap_now() { glfwSwapBuffers(window); };
void DesktopWindow::make_current() { glfwMakeContextCurrent(window); };

}  // namespace aardvark

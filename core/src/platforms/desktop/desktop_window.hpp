#pragma once

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "../../base_types.hpp"

namespace aardvark {

class DesktopWindow {
  public:
    DesktopWindow(Size size);
    ~DesktopWindow();
    // Disable copy and assignment
    DesktopWindow(const DesktopWindow&) = delete;
    DesktopWindow& operator=(DesktopWindow const&) = delete;
    Size size;
    void make_current();
    void swap_now();
    void swap();
    GLFWwindow* window;
};

}  // namespace aardvark

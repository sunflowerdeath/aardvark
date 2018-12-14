#pragma once

#include <unordered_map>
#include <variant>
#include <vector>
#include <functional>
#include <memory>
#include <GLFW/glfw3.h>
#include "base_types.hpp"
#include "window.hpp"
#include "document.hpp"
#include "compositing.hpp"

namespace aardvark {

struct WindowFocusEvent {};

struct WindowBlurEvent {};

struct WindowMoveEvent {
  WindowMoveEvent(double left, double top) : left(left), top(top){};
  double left;
  double top;
};

struct WindowCloseEvent {};
struct WindowMinimizeEvent {};
struct WindowRestoreEvent {};

using Event =
    std::variant<WindowFocusEvent, WindowBlurEvent, WindowMoveEvent,
                 WindowCloseEvent, WindowMinimizeEvent, WindowRestoreEvent>;
/*
enum class Events {
  WINDOW_FOCUS,
  WINDOW_BLUR,
  WINDOW_MOVE,
  WINDOW_CLOSE,
  WINDOW_MINIMIZE,
  WINDOW_RESTORE,
  MOUSE_ENTER,
  MOUSE_LEAVE,
  MOUSE_DOWN,
  MOUSE_UP,
  SCROLL
};
*/

class DesktopApp {
 public:
  // Runs application loop - polls events, calls handlers and repaints
  void run();
  // Stops application loop
  void stop();

  std::shared_ptr<DesktopWindow> create_window(Size size);
  void destroy_window(std::shared_ptr<DesktopWindow> window);
  Document* get_document(std::shared_ptr<DesktopWindow> window);

  // Used provided event handler
  std::function<void(DesktopApp* app, Event event)> event_handler;

  // Dispatches event to the corresponding App instance
  static void dispatch_event(GLFWwindow* window, Event event);

 private:
  bool should_stop;
  std::vector<std::shared_ptr<DesktopWindow>> windows;
  std::unordered_map<std::shared_ptr<DesktopWindow>, Document> documents;
  void handle_event(GLFWwindow* window, Event event);
};

}  // namespace aardvark
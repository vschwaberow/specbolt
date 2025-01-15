#include "X11MainWindow.hpp"

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/XKBlib.h>
#include <chrono>
#include <cstdint>
#include <format>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>
#include <sys/shm.h>

#include "ShmId.hpp"

namespace specbolt {

X11MainWindow::GraphicsContext::GraphicsContext(Display &display, Window window) : display(&display) {
  gc = XCreateGC(this->display, window, 0, nullptr);
  if (!gc) {
    throw std::runtime_error("Failed to create GC");
  }
}

X11MainWindow::GraphicsContext::~GraphicsContext() {
  if (display) {
    XFreeGC(display, gc);
  }
}

X11MainWindow::X11MainWindow(std::uint32_t width, std::uint32_t height)
    : width_(width), height_(height) {
  display_.reset(XOpenDisplay(nullptr));
  Bool bool_result{};
  XkbSetDetectableAutoRepeat(display_.get(), true, &bool_result);
  if (!bool_result) {
    throw std::runtime_error("XkbSetDetectableAutoRepeat failed");
  }
  const auto screen = DefaultScreen(display_.get());
  const auto depth = static_cast<unsigned int>(DefaultDepth(display_.get(), screen));
  if (depth != 24) {
    throw std::runtime_error(std::format("Invalid screen depth {}", depth));
  }
  const auto root_window = RootWindow(display_.get(), screen);
  const auto visual = DefaultVisual(display_.get(), screen);
  const auto black_pixel = BlackPixel(display_.get(), screen);
  const auto white_pixel = WhitePixel(display_.get(), screen);

  shm_id_ = ShmId(width_ * height_ * 4);
  image_data_ = static_cast<char *>(shmat(shm_id_.id(), nullptr, 0));

  image_ = XShmCreateImage(display_.get(), visual, depth, ZPixmap, nullptr, &shm_info_, width_, height_);
  if (!image_) {
    throw std::runtime_error("Unable to create shared memory image");
  }

  shm_info_.shmid = shm_id_.id();
  shm_info_.shmaddr = image_data_;
  shm_info_.readOnly = false;
  image_->data = image_data_;

  if (!XShmAttach(display_.get(), &shm_info_)) {
    throw std::runtime_error("XShmAttach failed");
  }

  static bool error_handler_called = false;
  static XErrorEvent last_error_event{};
  error_handler_called = false;
  const auto old_error_handler = XSetErrorHandler(+[](Display *, XErrorEvent *error_event) {
    if (!std::exchange(error_handler_called, true)) {
      last_error_event = *error_event;
    }
    return 0;
  });
  const auto sync_ret = XSync(display_.get(), false);
  XSetErrorHandler(old_error_handler);
  shm_id_.reset();
  if (!sync_ret) {
    throw std::runtime_error("XSync failed");
  }

  if (error_handler_called) {
    throw std::runtime_error(std::format("XShmAttach failed: error code {}", last_error_event.error_code));
  }

  window_ = XCreateSimpleWindow(display_.get(), root_window, 10, 10, width_, height_, 1, black_pixel, black_pixel);
  if (!window_) {
    throw std::runtime_error("Failed to create simple window");
  }

  SetWindowHints();

  atom_delete_message_ = XInternAtom(display_.get(), "WM_DELETE_WINDOW", false);
  if (const auto status_ret = XSetWMProtocols(display_.get(), window_, &atom_delete_message_, 1); status_ret == 0) {
    throw std::runtime_error("Failed to set WM protocols");
  }

  if (const auto res = XSelectInput(display_.get(), window_, KeyPressMask | KeyReleaseMask | FocusChangeMask);
      res == 0) {
    throw std::runtime_error("Failed to select input");
  }
  if (const auto res = XMapWindow(display_.get(), window_); res == 0) {
    throw std::runtime_error("Failed to map window");
  }
  gc_ = GraphicsContext(*display_, window_);
  if (const auto res = XSetBackground(display_.get(), gc_.gc, black_pixel); res == 0) {
    throw std::runtime_error("Failed to set background");
  }
  if (const auto res = XSetForeground(display_.get(), gc_.gc, white_pixel); res == 0) {
    throw std::runtime_error("Failed to set foreground");
  }
  if (const auto res = XFlush(display_.get()); res == 0) {
    throw std::runtime_error("XFlush failed");
  }

  if (const auto res = XStoreName(display_.get(), window_, "specbolt ZX Spectrum Emulator"); res == 0) {
    throw std::runtime_error("Failed to store name");
  }
}

void X11MainWindow::SetWindowHints() {
  XSizeHints hints;
  hints.flags = PPosition | PSize | PMinSize | PMaxSize;
  hints.x = 100;
  hints.y = 100;
  hints.width = width_;
  hints.height = height_;
  hints.min_width = 320;
  hints.min_height = 240;
  hints.max_width = 1920;
  hints.max_height = 1080;

  XSetWMNormalHints(display_.get(), window_, &hints);

  XWMHints wm_hints;
  wm_hints.flags = InputHint | StateHint;
  wm_hints.input = True;
  wm_hints.initial_state = NormalState;

  XSetWMHints(display_.get(), window_, &wm_hints);
}

void X11MainWindow::process_pending() {
  while (XPending(display_.get())) {
    XEvent event;
    XNextEvent(display_.get(), &event);
    switch (event.type) {
      case KeyPress:
        // TODO
        break;
      case KeyRelease:
        // TODO
        break;
      case FocusIn:
        // TODO
        break;
      case FocusOut:
        // TODO
        break;
      case ClientMessage:
        if (static_cast<Atom>(event.xclient.data.l[0]) == atom_delete_message_) {
          closed_ = true;
        }
        break;
      default:
        break;
    }
  }
}

void X11MainWindow::present_buffer() {
  if (!XShmPutImage(display_.get(), window_, gc_.gc, image_, 0, 0, 0, 0, width_, height_, false)) {
    throw std::runtime_error("XShmPutImage failed");
  }
  if (!XSync(display_.get(), false)) {
    throw std::runtime_error("XSync failed");
  }
  process_pending();
}

}

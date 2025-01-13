#include <X11/Xlib.h>

#include <X11/extensions/XShm.h>

#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <format>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "ShmId.hpp"

namespace specbolt {
class X11Window {
  std::uint32_t width_;
  std::uint32_t height_;

  struct XCloseDisplayer {
    void operator()(Display *display) const { XCloseDisplay(display); }
  };
  std::unique_ptr<Display, XCloseDisplayer> display_;
  ShmId shm_id_;
  char *image_data_{};
  XImage *image_{};
  XShmSegmentInfo shm_info_{};

  // TODO destruction of these:
  // Need to; free the GC, unmap the window, destroy the window and then detach the shm (seems the wrong wqay around)?
  // then destroy the XImage; then delete the shm...
  Window window_{};
  Atom atom_delete_message_{};
  GC gc_{};

  bool closed_{};

public:
  X11Window(const std::uint32_t width, const std::uint32_t height) : width_(width), height_(height) {
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

    shm_info_.shmid = shm_id_.id(); // mayyybe we 'release' here? TODO look into this
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
    shm_id_.reset(); // see above re: release
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
    gc_ = XCreateGC(display_.get(), window_, 0, nullptr);
    if (!gc_) {
      throw std::runtime_error("Failed to create GC");
    }
    if (const auto res = XSetBackground(display_.get(), gc_, black_pixel); res == 0) {
      throw std::runtime_error("Failed to set background");
    }
    if (const auto res = XSetForeground(display_.get(), gc_, white_pixel); res == 0) {
      throw std::runtime_error("Failed to set foreground");
    }
    if (const auto res = XFlush(display_.get()); res == 0) {
      throw std::runtime_error("XFlush failed");
    }

    if (const auto res = XStoreName(display_.get(), window_, "specbolt ZX Spectrum Emulator"); res == 0) {
      throw std::runtime_error("Failed to store name");
    }
  }

  [[nodiscard]] auto closed() const { return closed_; }
  [[nodiscard]] auto width() const { return width_; }
  [[nodiscard]] auto height() const { return height_; }
  [[nodiscard]] auto *image_data() const { return image_data_; }

  void process_pending() {
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

  void present_buffer() {
    if (!XShmPutImage(display_.get(), window_, gc_, image_, 0, 0, 0, 0, width_, height_, false)) {
      throw std::runtime_error("XShmPutImage failed");
    }
    if (!XSync(display_.get(), false)) {
      throw std::runtime_error("XSync failed");
    }
    // beebjit talks about checking for events here during rendering, and delays therein.
    process_pending();
  }
};

} // namespace specbolt

int main() {
  specbolt::X11Window window(640, 480);
  unsigned char rgb = 0xff;
  while (!window.closed()) {
    std::this_thread::sleep_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(16));
    memset(window.image_data(), rgb, window.width() * window.height() * 4);
    window.present_buffer();
    rgb++;
  }
}

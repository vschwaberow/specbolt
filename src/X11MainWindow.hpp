#pragma once

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>

#include "ShmId.hpp"

namespace specbolt {

class X11MainWindow {
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

  Window window_{};
  Atom atom_delete_message_{};

  struct GraphicsContext {
    Display *display{};
    GC gc{};
    GraphicsContext() = default;
    GraphicsContext(Display &display, Window window);
    ~GraphicsContext();
    GraphicsContext(const GraphicsContext &) = delete;
    GraphicsContext &operator=(const GraphicsContext &) = delete;
    GraphicsContext(GraphicsContext &&other) noexcept : display(std::exchange(other.display, nullptr)), gc(other.gc) {}
    GraphicsContext &operator=(GraphicsContext &&other) noexcept {
      if (this != &other) {
        display = std::exchange(other.display, nullptr);
        gc = other.gc;
      }
      return *this;
    }
  };
  GraphicsContext gc_{};

  bool closed_{};

  void SetWindowHints();

public:
  X11MainWindow(std::uint32_t width, std::uint32_t height);

  X11MainWindow(const X11MainWindow &) = delete;
  X11MainWindow &operator=(const X11MainWindow &) = delete;

  [[nodiscard]] auto closed() const { return closed_; }
  [[nodiscard]] auto width() const { return width_; }
  [[nodiscard]] auto height() const { return height_; }
  [[nodiscard]] auto *image_data() const { return image_data_; }

  void process_pending();
  void present_buffer();
};

}

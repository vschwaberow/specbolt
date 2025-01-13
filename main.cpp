#include <X11/Xlib.h>

#include <X11/extensions/XShm.h>

#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <cstdint>
#include <format>
#include <memory>
#include <stdexcept>

#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

class X11Window {
  std::uint32_t width_;
  std::uint32_t height_;

  struct XCloseDisplayer {
    void operator()(Display *display) const { XCloseDisplay(display); }
  };
  std::unique_ptr<Display, XCloseDisplayer> display_;
  int shm_id_{}; // TODO wrap in a smart pointer, rm_shmid
  char *image_data_{};
  XImage *image_{};
  XShmSegmentInfo shm_info_{};

  // TODO destruction of these:
  Window window_;
  Atom atom_delete_message_;
  GC gc_;

public:
  X11Window(std::uint32_t width, std::uint32_t height) : width_(width), height_(height) {
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

    shm_id_ = shmget(IPC_PRIVATE, width_ * height_ * 4, IPC_CREAT | 0600);
    if (shm_id_ < 0) {
      throw std::runtime_error(std::format("Failed to create shared memory: {}", errno));
    }
    image_data_ = static_cast<char *>(shmat(shm_id_, nullptr, 0));

    image_ = XShmCreateImage(display_.get(), visual, depth, ZPixmap, image_data_, &shm_info_, width_, height_);
    if (!image_) {
      throw std::runtime_error("Unable to create shared memory image");
    }

    shm_info_.shmid = shm_id_;
    shm_info_.shmaddr = image_data_;
    shm_info_.readOnly = false;
    image_->data = image_data_;

    if (!XShmAttach(display_.get(), &shm_info_)) {
      throw std::runtime_error("XShmAttach failed");
    }

    // something with error handlers...
    const auto sync_ret = XSync(display_.get(), false);
    // something

    ///// probably not needed if we smart wrapper the shnid.
    // rm_shmid();
    shmctl(shm_id_, IPC_RMID, nullptr);
    shm_id_ = -1;
    /////

    if (sync_ret != 1) {
      throw std::runtime_error("XSync failed");
    }

    // something with last error handlers...

    window_ = XCreateSimpleWindow(display_.get(), root_window, 0, 0, width_, height_, 1, black_pixel, black_pixel);
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
  }
};


int main() {
  X11Window window(640, 480);
  while (true) {}
}

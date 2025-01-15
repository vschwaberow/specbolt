#include <chrono>
#include <cstring>
#include <thread>
#include "X11MainWindow.hpp"

int main() {
  specbolt::X11MainWindow window(640, 480);
  unsigned char rgb = 0xff;
  while (!window.closed()) {
    const auto next_frame = std::chrono::steady_clock::now() + std::chrono::milliseconds(20);
    // EMULATE HERE
    memset(window.image_data(), rgb, window.width() * window.height() * 4);
    std::this_thread::sleep_until(next_frame); // consider not sleeping but busy-waiting...
    window.present_buffer();
    rgb++;
  }
}

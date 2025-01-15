#include <chrono>
#include <cstring>
#include <thread>
#include "X11MainWindow.hpp"
#include "Logger.hpp"

int main() {
  auto& logger = specbolt::Logger::Instance();
  logger.SetLogLevel(specbolt::LogLevel::DEBUG);
  logger.SetLogFile("specbolt.log");
  logger.SetLogFormat("%Y-%m-%d %X [LEVEL] MESSAGE");

  logger.Log(specbolt::LogLevel::INFO, "Starting the ZX Spectrum emulator", "main", "main");

  specbolt::X11MainWindow window(640, 480);
  logger.Log(specbolt::LogLevel::INFO, "Created X11 main window", "main", "main");

  unsigned char rgb = 0xff;
  while (!window.closed()) {
    const auto next_frame = std::chrono::steady_clock::now() + std::chrono::milliseconds(20);

    // EMULATE HERE
    memset(window.image_data(), rgb, window.width() * window.height() * 4);
    logger.Log(specbolt::LogLevel::DEBUG, "Updated image data", "main", "main");

    std::this_thread::sleep_until(next_frame); // consider not sleeping but busy-waiting...
    window.present_buffer();
    logger.Log(specbolt::LogLevel::DEBUG, "Presented buffer", "main", "main");

    rgb++;
  }

  logger.Log(specbolt::LogLevel::INFO, "Exiting the ZX Spectrum emulator", "main", "main");
  return 0;
}

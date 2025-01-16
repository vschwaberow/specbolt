#include "ShmId.hpp"

#include <format>
#include <stdexcept>
#include <sys/shm.h>

specbolt::ShmId::ShmId(const std::size_t num_bytes) : size_(num_bytes) {
  id_ = shmget(IPC_PRIVATE, num_bytes, IPC_CREAT | 0600);
  if (id_ < 0) {
    throw std::runtime_error(std::format("Failed to create shared memory: {}", errno));
  }
  data_ = static_cast<char *>(shmat(id_, nullptr, 0));
  if (data_ == reinterpret_cast<char *>(-1)) {
    throw std::runtime_error(std::format("Failed to attach shared memory: {}", errno));
  }
}

void specbolt::ShmId::reset() {
  if (data_ && data_ != reinterpret_cast<char *>(-1)) {
    if (shmdt(data_) == -1) {
      throw std::runtime_error(std::format("Failed to detach shared memory: {}", std::strerror(errno)));
    }
    data_ = nullptr;
  }
  if (id_ != -1) {
    if (shmctl(id_, IPC_RMID, nullptr) == -1) {
      throw std::runtime_error(std::format("Failed to mark shared memory for destruction: {}", std::strerror(errno)));
    }
    id_ = -1;
  }
  size_ = 0;
}

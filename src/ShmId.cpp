#include "ShmId.hpp"

#include <format>
#include <stdexcept>
#include <sys/shm.h>

specbolt::ShmId::ShmId(const std::size_t num_bytes) : size_(num_bytes) {
  id_ = shmget(IPC_PRIVATE, num_bytes, IPC_CREAT | 0600);
  if (id_ < 0) {
    throw std::runtime_error(std::format("Failed to create shared memory: {}", errno));
  }
  data_ = static_cast<char*>(shmat(id_, nullptr, 0));
  if (data_ == reinterpret_cast<char*>(-1)) {
    throw std::runtime_error(std::format("Failed to attach shared memory: {}", errno));
  }
}

void specbolt::ShmId::reset() {
  if (data_ != nullptr && data_ != reinterpret_cast<char*>(-1)) {
    shmdt(data_);
    data_ = nullptr;
  }
  if (id_ != -1) {
    shmctl(id_, IPC_RMID, nullptr);
    id_ = -1;
  }
  size_ = 0;
}

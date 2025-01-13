#include "ShmId.hpp"

#include <format>
#include <stdexcept>
#include <sys/shm.h>

specbolt::ShmId::ShmId(const std::size_t num_bytes) {
  id_ = shmget(IPC_PRIVATE, num_bytes, IPC_CREAT | 0600);
  if (id_ < 0) {
    throw std::runtime_error(std::format("Failed to create shared memory: {}", errno));
  }
}

void specbolt::ShmId::reset() {
  if (id_ != -1)
    shmctl(id_, IPC_RMID, nullptr);
  // TODO decide what to do if shmctl fails.
  id_ = -1;
}

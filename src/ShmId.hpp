#pragma once

#include <cstddef>

namespace specbolt {

/// Holder for a shared memory id.
class ShmId {
  int id_{-1};

public:
  ShmId() = default;
  explicit ShmId(std::size_t num_bytes);
  ShmId(const ShmId &) = delete;
  ShmId &operator=(const ShmId &) = delete;
  ShmId(ShmId &&other) noexcept : id_(other.id_) { other.id_ = -1; }
  ShmId &operator=(ShmId &&other) noexcept {
    if (this != &other) {
      id_ = other.id_;
      other.id_ = -1;
    }
    return *this;
  }
  ~ShmId() { reset(); }
  int id() const { return id_; }
  void reset();
};

} // namespace specbolt

#pragma once

#include <cstddef>

namespace specbolt {

/// Holder for a shared memory id.
class ShmId {
  int id_{-1};
  std::size_t size_{0};
  char* data_{nullptr};

public:
  ShmId() = default;
  explicit ShmId(std::size_t num_bytes);
  ShmId(const ShmId &) = delete;
  ShmId &operator=(const ShmId &) = delete;
  ShmId(ShmId &&other) noexcept : id_(other.id_), size_(other.size_), data_(other.data_) {
    other.id_ = -1;
    other.size_ = 0;
    other.data_ = nullptr;
  }
  ShmId &operator=(ShmId &&other) noexcept {
    if (this != &other) {
      id_ = other.id_;
      size_ = other.size_;
      data_ = other.data_;
      other.id_ = -1;
      other.size_ = 0;
      other.data_ = nullptr;
    }
    return *this;
  }
  ~ShmId() { reset(); }
  int id() const { return id_; }
  void reset();
  char* Data() const { return data_; }
  std::size_t Size() const { return size_; }
};

} // namespace specbolt

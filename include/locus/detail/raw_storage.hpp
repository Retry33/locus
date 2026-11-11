#ifndef LOCUS_DETAIL_RAW_STORAGE_HPP_
#define LOCUS_DETAIL_RAW_STORAGE_HPP_

#include <cstddef>
#include <memory>
#include <utility>

namespace locus::detail {
template <typename T>
class RawStorage {
 public:
  RawStorage() = default;
  explicit RawStorage(std::size_t n)
      : data_(n == 0 ? nullptr : std::allocator<T>{}.allocate(n)),
        capacity_(n) {}

  RawStorage(const RawStorage& other) = delete;
  RawStorage& operator=(const RawStorage& other) = delete;

  RawStorage(RawStorage&& other) noexcept
      : data_(std::exchange(other.data_, nullptr)),
        capacity_(std::exchange(other.capacity_, 0)) {}

  RawStorage& operator=(RawStorage&& other) noexcept {
    RawStorage tmp(std::move(other));
    swap(tmp);
    return *this;
  }

  ~RawStorage() {
    if (data_ != nullptr) {
      std::allocator<T>{}.deallocate(data_, capacity_);
    }
  }

  void swap(RawStorage& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(capacity_, other.capacity_);
  }

  [[nodiscard]] T* data() const noexcept { return data_; }
  [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

 private:
  T* data_ = nullptr;
  std::size_t capacity_ = 0;
};
}  // namespace locus::detail

#endif
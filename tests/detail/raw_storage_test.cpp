#include "locus/detail/raw_storage.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

namespace {

using locus::detail::RawStorage;

static_assert(!std::is_copy_constructible_v<RawStorage<int>>,
              "RawStorage must not be copy-constructible");
static_assert(!std::is_copy_assignable_v<RawStorage<int>>,
              "RawStorage must not be copy-assignable");
static_assert(std::is_nothrow_move_constructible_v<RawStorage<int>>,
              "RawStorage must be nothrow move-constructible");
static_assert(std::is_nothrow_move_assignable_v<RawStorage<int>>,
              "RawStorage must be nothrow move-assignable");
static_assert(!std::is_convertible_v<std::size_t, RawStorage<int>>,
              "RawStorage must not be implicitly convertible from a size");
static_assert(std::is_constructible_v<RawStorage<int>, std::size_t>,
              "RawStorage must be constructible from a size");

TEST(RawStorage, DefaultConstructedIsEmpty) {
  RawStorage<int> s;
  EXPECT_EQ(s.data(), nullptr);
  EXPECT_EQ(s.capacity(), 0u);
}

class RawStorageCapacity : public ::testing::TestWithParam<std::size_t> {};

TEST_P(RawStorageCapacity, StoresRequestedCapacity) {
  // Arrange
  const std::size_t n = GetParam();

  // Act
  RawStorage<int> s(n);

  // Assert
  EXPECT_EQ(s.capacity(), n);
  EXPECT_EQ(s.data() == nullptr, n == 0);
}

TEST_P(RawStorageCapacity, LastSlotIsUsable) {
  // Arrange
  const std::size_t n = GetParam();
  if (n == 0) {
    GTEST_SKIP();
  }

  RawStorage<int> s(n);
  ASSERT_NE(s.data(), nullptr);

  // Act
  int* const last = std::construct_at(s.data() + n - 1, 67);

  // Assert
  EXPECT_EQ(*last, 67);

  // Cleanup
  std::destroy_at(last);
}

INSTANTIATE_TEST_SUITE_P(Sizes, RawStorageCapacity,
                         ::testing::Values(0u, 1u, 7u, 64u, 1u << 20));

class RawStorageMove : public ::testing::Test {
 protected:
  RawStorage<int> full_{16};
  RawStorage<int> empty_;
};

TEST_F(RawStorageMove, MoveConstructorTransfersOwnership) {
  // Arrange
  int* const ptr = full_.data();

  // Act
  RawStorage<int> buf(std::move(full_));

  // Assert
  EXPECT_EQ(buf.data(), ptr);
  EXPECT_EQ(buf.capacity(), 16u);
  EXPECT_EQ(full_.data(), nullptr);
  EXPECT_EQ(full_.capacity(), 0u);
}

TEST_F(RawStorageMove, MoveAssignmentIntoNonEmptyTakesOwnership) {
  // Arrange
  int* const ptr = full_.data();
  RawStorage<int> target(4);

  // Act
  target = std::move(full_);

  // Assert
  EXPECT_EQ(target.data(), ptr);
  EXPECT_EQ(target.capacity(), 16u);
  EXPECT_EQ(full_.data(), nullptr);
  EXPECT_EQ(full_.capacity(), 0u);
}

TEST_F(RawStorageMove, SelfMoveAssignmentKeepsState) {
  // Arrange
  int* const ptr = full_.data();
  RawStorage<int>& alias = full_;

  // Act
  full_ = std::move(alias);

  // Assert
  EXPECT_EQ(full_.data(), ptr);
  EXPECT_EQ(full_.capacity(), 16u);
}

TEST_F(RawStorageMove, SwapExchangesState) {
  // Arrange
  RawStorage<int> other(4);
  int* const full_ptr = full_.data();
  int* const other_ptr = other.data();

  // Act
  full_.swap(other);

  // Assert
  EXPECT_EQ(full_.data(), other_ptr);
  EXPECT_EQ(full_.capacity(), 4u);
  EXPECT_EQ(other.data(), full_ptr);
  EXPECT_EQ(other.capacity(), 16u);
}

TEST_F(RawStorageMove, SwapWithEmptyTransfersOwnership) {
  // Arrange
  int* const ptr = full_.data();

  // Act
  full_.swap(empty_);

  // Assert
  EXPECT_EQ(empty_.data(), ptr);
  EXPECT_EQ(empty_.capacity(), 16u);
  EXPECT_EQ(full_.data(), nullptr);
  EXPECT_EQ(full_.capacity(), 0u);
}

TEST_F(RawStorageMove, SelfSwapKeepsState) {
  // Arrange
  int* const ptr = full_.data();

  // Act
  full_.swap(full_);

  // Assert
  EXPECT_EQ(full_.data(), ptr);
  EXPECT_EQ(full_.capacity(), 16u);
}

TEST(RawStorage, HugeAllocationThrows) {
  // Arrange / Act / Assert
  EXPECT_THROW(RawStorage<int>{std::numeric_limits<std::size_t>::max()},
               std::bad_alloc);
}

struct alignas(64) CacheLineAligned {
  char c;
};

struct NoDefault {
  explicit NoDefault(int);
};

template <typename T>
class RawStorageTyped : public ::testing::Test {};

using StorageTypes = ::testing::Types<char, int, double, std::string,
                                      CacheLineAligned, NoDefault>;

TYPED_TEST_SUITE(RawStorageTyped, StorageTypes);

TYPED_TEST(RawStorageTyped, RespectsAlignment) {
  // Arrange / Act
  RawStorage<TypeParam> s(3u);

  // Assert
  ASSERT_NE(s.data(), nullptr);
  const auto addr = reinterpret_cast<std::uintptr_t>(s.data());
  EXPECT_EQ(addr % alignof(TypeParam), 0u);
}

}  // namespace
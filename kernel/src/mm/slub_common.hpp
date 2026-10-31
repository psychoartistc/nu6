#pragma once

#include <atomic>
#include <cstddef>
#include <concepts>
#include <cstdint>

namespace mm {
namespace slub {

template <typename A>
concept slab_backend = requires(A a, void *p) {
  { a.allocate() } -> std::convertible_to<void *>;
  a.deallocate(p);
};

template <std::size_t slabSize, slab_backend Alloc> class slub_allocator;

inline constexpr std::uint32_t SLUB_MAGIC_SLAB = 0x534c4142;  // 'SLAB'
inline constexpr std::uint32_t SLUB_MAGIC_LARGE = 0x4c415247; // 'LARG'

struct slub_header {
  std::uint32_t magic;
  std::uint32_t class_idx;
  std::uint64_t alloc_size;
};
static_assert(sizeof(slub_header) == 16);

struct slub_slab {
  slub_slab *all_next;
  slub_slab *spare_next;
  std::uint8_t *base;
  std::uint32_t capacity;
  std::atomic<std::uint32_t> bump_index;
};

inline constexpr unsigned SLUB_PTR_BITS = 48;
inline constexpr std::uint64_t SLUB_PTR_MASK =
    (std::uint64_t(1) << SLUB_PTR_BITS) - 1;
inline constexpr std::uint64_t SLUB_SIGN_BIT = std::uint64_t(1)
                                               << (SLUB_PTR_BITS - 1);

constexpr std::size_t align16(std::size_t v) {
  return (v + 15u) & ~std::size_t(15u);
}

inline std::uint64_t pack_pointer(void *ptr, std::uint16_t tag) {
  auto p = reinterpret_cast<std::uint64_t>(ptr) & SLUB_PTR_MASK;
  return p | (static_cast<std::uint64_t>(tag) << SLUB_PTR_BITS);
}

inline void *unpack_pointer(std::uint64_t packed) {
  std::uint64_t p = packed & SLUB_PTR_MASK;
  if (p & SLUB_SIGN_BIT)
    p |= ~SLUB_PTR_MASK;
  return reinterpret_cast<void *>(p);
}

inline std::uint16_t unpack_tag(std::uint64_t packed) {
  return static_cast<std::uint16_t>(packed >> SLUB_PTR_BITS);
}

inline void free_slot_set_next(void *node, void *next) {
  *static_cast<void **>(node) = next;
}

inline void *free_slot_get_next(void *node) {
  return *static_cast<void **>(node);
}
} // namespace slub
//} // namespace slub
} // namespace mm

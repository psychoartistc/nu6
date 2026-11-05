#pragma once

#include <utils/math.hpp>
#include <atomic>
#include <cstddef>
#include <concepts>
#include <cstdint>

namespace mm {
namespace slub {
template <typename A>
concept fixed_slab_backend = requires(A a, void *p) {
  { a.allocate() } -> std::convertible_to<void *>;
  a.deallocate(p);
};

template <typename A>
concept dynamic_slab_backend = requires(A a, void *p, std::size_t n) {
  { a.allocate(n) } -> std::convertible_to<void *>;
  a.deallocate(p, n);
};

template <typename A, bool dynamic>
concept slab_backend =
    (dynamic && dynamic_slab_backend<A>) || (!dynamic && fixed_slab_backend<A>);

template <std::size_t slabSize, typename Alloc, bool dynamicBackend>
  requires slab_backend<Alloc, dynamicBackend>
class slub_allocator;

inline constexpr std::uint32_t SLUB_MAGIC_SLAB = 0x534c4142;  // 'SLAB'
inline constexpr std::uint32_t SLUB_MAGIC_LARGE = 0x4c415247; // 'LARG'

struct slub_header {
  std::uint32_t magic;
  std::uint32_t class_idx;
  std::uint64_t alloc_size;
};

struct slub_slab {
  slub_slab *all_next;
  slub_slab *spare_next;
  std::uint8_t *base;
  std::uint32_t capacity;
  std::atomic<std::uint32_t> bump_index;
};

static inline constexpr std::size_t align16(std::size_t v) {
  return utils::align_up<std::size_t>(v, 16);
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

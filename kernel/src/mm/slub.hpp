#pragma once

#include <algorithm>
#include <array>
#include <limits>
#include <mm/slub_cache.hpp>
#include <string_view>
#include <utility>

namespace mm {

namespace slub {
template <std::size_t slabSize, slab_backend Alloc> class slub_allocator {

  using cache_type = slub_cache_impl<slabSize, Alloc>;
  static constexpr std::size_t HEADER_SIZE = sizeof(slub_header);

public:
  static constexpr std::array<std::size_t, 8> CLASS_SIZES{16,  32,  64,   128,
                                                          256, 512, 1024, 2048};

  static constexpr std::array<std::string_view, 8> CLASS_NAMES{
      "slub_16",  "slub_32",  "slub_64",   "slub_128",
      "slub_256", "slub_512", "slub_1024", "slub_2048"};

  static constexpr std::size_t NUM_CLASSES = CLASS_SIZES.size();

  static_assert(cache_type::slab_capacity_for(slub::align16(CLASS_SIZES.back() +
                                                            HEADER_SIZE)) > 0,
                "slabSize too small for the largest size class");

  static slub_allocator &get() {
    static slub_allocator s;
    return s;
  }

  slub_allocator(const slub_allocator &) = delete;
  slub_allocator &operator=(const slub_allocator &) = delete;

  void *allocate(std::size_t size) {
    if (!size)
      return nullptr;

    int index = size_to_class(size);
    if (index >= 0) {
      return m_caches[index].allocate_raw(static_cast<std::uint32_t>(index),
                                          size);
    }

    if (size > std::numeric_limits<std::size_t>::max() - HEADER_SIZE)
      return nullptr;

    auto total = size + HEADER_SIZE;
    if (total > slabSize)
      return nullptr;

    void *memory = m_large.allocate();
    if (!memory)
      return nullptr;

    auto *header = static_cast<slub_header *>(memory);
    header->magic = SLUB_MAGIC_LARGE;
    header->class_idx = 0;
    header->alloc_size = total;
    return static_cast<std::uint8_t *>(memory) + HEADER_SIZE;
  }

  void free(void *ptr) {
    if (!ptr)
      return;

    auto *header = to_header(ptr);

    if (header->magic == SLUB_MAGIC_LARGE) {
      header->magic = 0;
      m_large.deallocate(header);
      return;
    }

    if (header->magic != SLUB_MAGIC_SLAB || header->class_idx >= NUM_CLASSES)
      return;

    m_caches[header->class_idx].free_raw(header);
  }

  void *reallocate(void *ptr, std::size_t size) {
    if (!ptr)
      return allocate(size);
    if (!size) {
      free(ptr);
      return nullptr;
    }

    auto *header = to_header(ptr);

    std::size_t old_size;
    if (header->magic == SLUB_MAGIC_LARGE)
      old_size = slabSize - HEADER_SIZE;
    else if (header->magic == SLUB_MAGIC_SLAB &&
             header->class_idx < NUM_CLASSES)
      old_size = CLASS_SIZES[header->class_idx];
    else
      return nullptr;

    if (size <= old_size)
      return ptr;

    void *replacement = allocate(size);
    if (!replacement)
      return nullptr;

    memcpy(replacement, ptr, old_size);
    free(ptr);
    return replacement;
  }

private:
  slub_allocator() : slub_allocator(std::make_index_sequence<NUM_CLASSES>{}) {}

  template <std::size_t... I>
  explicit slub_allocator(std::index_sequence<I...>)
      : m_caches{cache_type(CLASS_NAMES[I], CLASS_SIZES[I])...} {}

  static slub_header *to_header(void *ptr) {
    return reinterpret_cast<slub_header *>(static_cast<std::uint8_t *>(ptr) -
                                           HEADER_SIZE);
  }

  static constexpr int size_to_class(std::size_t size) {
    std::size_t left = 0, right = NUM_CLASSES;
    while (left < right) {
      auto mid = left + (right - left) / 2;
      if (size <= CLASS_SIZES[mid])
        right = mid;
      else
        left = mid + 1;
    }
    return left < NUM_CLASSES ? static_cast<int>(left) : -1;
  }

  std::array<cache_type, NUM_CLASSES> m_caches;
  Alloc m_large;
};

} // namespace slub

template <typename T, std::size_t slabSize, typename backend>
  requires slub::slab_backend<backend>
class slub_stl_allocator {
  using base_allocator = slub::slub_allocator<slabSize, backend>;

  static_assert(alignof(T) <= 16, "over aligned types are not supported");

public:
  using value_type = T;
  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::true_type;

  template <typename U> struct rebind {
    using other = slub_stl_allocator<U, slabSize, backend>;
  };

  slub_stl_allocator() noexcept = default;

  template <typename U>
  slub_stl_allocator(
      const slub_stl_allocator<U, slabSize, backend> &) noexcept {}

  [[nodiscard]] T *allocate(std::size_t n) {
    if (n == 0 || n > std::numeric_limits<std::size_t>::max() / sizeof(T))
      return nullptr;
    return static_cast<T *>(base_allocator::get().allocate(sizeof(T) * n));
  }

  void deallocate(T *p, std::size_t) noexcept { base_allocator::get().free(p); }
};

template <typename T, typename U, std::size_t slabSize, typename backend>
constexpr bool
operator==(const slub_stl_allocator<T, slabSize, backend> &,
           const slub_stl_allocator<U, slabSize, backend> &) noexcept {
  return true;
}
} // namespace mm

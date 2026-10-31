#pragma once

#include <std/memory.hpp>
#include <arch/smp/core_manager.hpp>
#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>

#include <mm/slub_common.hpp>
#include <string_view>
#include <utility>

namespace mm {

namespace slub {
template <std::size_t slabSize, typename Alloc> class slub_cache_impl {
  static_assert(slab_backend<Alloc>,
                "Alloc needs void* allocate() and void deallocate(void*)");

  static constexpr std::size_t HEADER_SIZE = sizeof(slub_header);
  static constexpr std::size_t SLAB_HEADER_AREA =
      slub::align16(sizeof(slub_slab));
  static_assert(slabSize > SLAB_HEADER_AREA + HEADER_SIZE,
                "slabSize too small");

public:
  static constexpr std::size_t slab_capacity_for(std::size_t slot_size) {
    if (slot_size == 0 || slot_size > slabSize - SLAB_HEADER_AREA)
      return 0;

    return (slabSize - SLAB_HEADER_AREA) / slot_size;
  }

  slub_cache_impl(std::string_view name, std::size_t object_size,
                  Alloc alloc = Alloc{})
      : m_name(name), m_objectSize(object_size),
        m_slotSize(object_size > slabSize
                       ? 0
                       : slub::align16(object_size + HEADER_SIZE)),
        m_capacity(slab_capacity_for(m_slotSize)), m_alloc(std::move(alloc)) {
    for (auto &slab : m_cpuSlabs)
      slab.store(nullptr, std::memory_order_relaxed);
  }

  ~slub_cache_impl() {
    m_destroying.store(true, std::memory_order_seq_cst);
    while (m_activeOps.load(std::memory_order_acquire) != 0)
      arch::pause();

    release_slabs();
  }

  slub_cache_impl(const slub_cache_impl &) = delete;
  slub_cache_impl &operator=(const slub_cache_impl &) = delete;

  void *allocate() { return allocate_raw(0, m_objectSize); }

  void *zallocate() {
    void *ptr = allocate();
    if (ptr)
      memset(ptr, 0, m_objectSize);

    return ptr;
  }

  void free(void *ptr) {
    if (!ptr)
      return;
    auto *header = reinterpret_cast<slub_header *>(
        static_cast<std::uint8_t *>(ptr) - HEADER_SIZE);
    free_raw(header);
  }

private:
  friend class slub_allocator<slabSize, Alloc>;

  bool enter() {
    m_activeOps.fetch_add(1, std::memory_order_seq_cst);

    if (m_destroying.load(std::memory_order_seq_cst)) {
      m_activeOps.fetch_sub(1, std::memory_order_release);
      return false;
    }
    return true;
  }

  void exit() { m_activeOps.fetch_sub(1, std::memory_order_release); }

  void *allocate_raw(std::uint32_t class_index, std::size_t requested_size) {
    // pr_info("allocate_raw in");
    if (m_capacity == 0 || !enter()) {
      // pr_info("failed to enter");
      return nullptr;
    }

    // pr_info("popping object");
    void *slot = free_object_pop();
    if (!slot) {
      // pr_info("pop failed, bump allocating");
      slot = bump_allocate();
    }

    // pr_info("allocated");

    if (!slot) {
      // pr_info("!slot");
      exit();
      return nullptr;
    }

    // pr_info("header writing");
    auto *header = static_cast<slub_header *>(slot);
    header->magic = SLUB_MAGIC_SLAB;
    header->class_idx = class_index;
    header->alloc_size = requested_size;
    // pr_info("header written");

    exit();
    // pr_info("exited");
    return static_cast<std::uint8_t *>(slot) + HEADER_SIZE;
  }

  void free_raw(slub_header *header) {
    if (header->magic != SLUB_MAGIC_SLAB || !enter())
      return;

    header->magic = 0;
    free_object_push(header);
    exit();
  }

  void *bump_allocate() {
    auto &cell = m_cpuSlabs[arch::smp::core::current().index()];
    for (;;) {
      auto *slab = cell.load(std::memory_order_acquire);
      if (slab &&
          slab->bump_index.load(std::memory_order_relaxed) < slab->capacity) {
        auto index = slab->bump_index.fetch_add(1, std::memory_order_relaxed);
        if (index < slab->capacity)
          return slab->base + std::size_t(index) * m_slotSize;
      }

      auto *fresh = acquire_slab();
      if (!fresh)
        return nullptr;

      auto *expected = slab;
      if (!cell.compare_exchange_strong(expected, fresh,
                                        std::memory_order_acq_rel,
                                        std::memory_order_acquire))
        spare_slab_push(fresh);
    }
  }

  slub_slab *acquire_slab() {
    if (auto *spare = spare_slab_pop())
      return spare;

    void *memory = m_alloc.allocate();
    if (!memory)
      return nullptr;

    auto *slab = static_cast<slub_slab *>(memory);
    slab->spare_next = nullptr;
    slab->base = static_cast<std::uint8_t *>(memory) + SLAB_HEADER_AREA;
    slab->capacity = static_cast<std::uint32_t>(m_capacity);
    slab->bump_index.store(0, std::memory_order_relaxed);

    slab_list_push(slab);
    return slab;
  }

  void release_slabs() {
    auto *slab = m_allSlabs.exchange(nullptr, std::memory_order_acq_rel);
    while (slab) {
      auto *next = slab->all_next;
      m_alloc.deallocate(slab);
      slab = next;
    }
  }

  void free_object_push(void *node) {
    using namespace slub;
    auto current = m_freeStack.load(std::memory_order_acquire);

    for (;;) {
      free_slot_set_next(node, unpack_pointer(current));
      auto next = pack_pointer(node, unpack_tag(current) + 1);
      if (m_freeStack.compare_exchange_weak(current, next,
                                            std::memory_order_acq_rel,
                                            std::memory_order_acquire))
        return;
    }
  }

  void *free_object_pop() {
    using namespace slub;
    auto current = m_freeStack.load(std::memory_order_acquire);
    for (;;) {
      void *top = unpack_pointer(current);
      if (!top)
        return nullptr;

      auto next =
          pack_pointer(free_slot_get_next(top), unpack_tag(current) + 1);

      if (m_freeStack.compare_exchange_weak(current, next,
                                            std::memory_order_acq_rel,
                                            std::memory_order_acquire))
        return top;
    }
  }

  void slab_list_push(slub_slab *slab) {
    auto current = m_allSlabs.load(std::memory_order_relaxed);
    do {
      slab->all_next = current;
    } while (!m_allSlabs.compare_exchange_weak(
        current, slab, std::memory_order_release, std::memory_order_relaxed));
  }

  void spare_slab_push(slub_slab *slab) {
    using namespace slub;
    auto current = m_spareSlabs.load(std::memory_order_acquire);

    for (;;) {
      slab->spare_next = static_cast<slub_slab *>(unpack_pointer(current));
      auto next = pack_pointer(slab, unpack_tag(current) + 1);
      if (m_spareSlabs.compare_exchange_weak(current, next,
                                             std::memory_order_acq_rel,
                                             std::memory_order_acquire))
        return;
    }
  }

  slub_slab *spare_slab_pop() {
    using namespace slub;
    auto current = m_spareSlabs.load(std::memory_order_acquire);
    for (;;) {
      auto *top = static_cast<slub_slab *>(unpack_pointer(current));
      if (!top)
        return nullptr;

      auto next = pack_pointer(top->spare_next, unpack_tag(current) + 1);
      if (m_spareSlabs.compare_exchange_weak(current, next,
                                             std::memory_order_acq_rel,
                                             std::memory_order_acquire))
        return top;
    }
  }

  std::string_view m_name;
  slub_cache_impl *m_next = nullptr;
  std::uint64_t m_id = 0;
  std::size_t m_objectSize;
  std::size_t m_slotSize;
  std::size_t m_capacity;
  Alloc m_alloc;

  std::atomic<std::uint64_t> m_freeStack{0};
  std::atomic<std::uint64_t> m_spareSlabs{0};
  std::atomic<slub_slab *> m_allSlabs{nullptr};

  std::atomic<std::uint32_t> m_activeOps{0};
  std::atomic<bool> m_destroying{false};

  std::array<std::atomic<slub_slab *>, arch::smp::MAX_CORES> m_cpuSlabs{};
};

template <typename T, std::size_t slabSize, typename Alloc> class slub_cache {
  // friend class slub_allocator;

public:
  explicit slub_cache(std::string_view name, Alloc alloc = Alloc{})
      : m_cache(name, sizeof(T), std::move(alloc)) {}

  slub_cache(std::string_view name, std::size_t object_size,
             Alloc alloc = Alloc{})
      : m_cache(name, object_size, std::move(alloc)) {}

  slub_cache(const slub_cache &) = delete;
  slub_cache &operator=(const slub_cache &) = delete;

  [[nodiscard]] T *allocate() { return static_cast<T *>(m_cache.allocate()); }
  [[nodiscard]] T *zallocate() { return static_cast<T *>(m_cache.zallocate()); }
  void free(T *ptr) { m_cache.free(ptr); }

private:
  slub_cache_impl<slabSize, Alloc> m_cache;
};

} // namespace slub
} // namespace mm

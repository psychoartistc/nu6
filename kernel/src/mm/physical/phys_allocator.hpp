#pragma once

#include <utils/panic.hpp>
#include <mm/physical/page_allocator.hpp>
#include <utils/math.hpp>

namespace mm {
class page_pool {
  struct page_header {
    page_header *all_prev, *all_next;
    page_header *part_prev, *part_next;
    void *free_list;
    std::uint32_t bump;
    std::uint32_t used;
    std::uintptr_t phys;
    bool in_partial;
  };

  page_header *m_all = nullptr;
  page_header *m_partial = nullptr;
  std::size_t m_empty = 0;

  std::size_t m_slotSize = 0, m_slotAlign = 0, m_firstSlot = 0, m_capacity = 0;

private:
  void bind(std::size_t size, std::size_t align);

  void *slot_at(page_header *pg, std::size_t i) const noexcept {
    return reinterpret_cast<char *>(pg) + m_firstSlot + i * m_slotSize;
  }

  void partial_push(page_header *pg) noexcept;
  void partial_remove(page_header *pg) noexcept;
  page_header *new_page();
  void release_page(page_header *pg) noexcept;

public:
  page_pool() noexcept = default;
  page_pool(const page_pool &) = delete;
  page_pool &operator=(const page_pool &) = delete;

  ~page_pool();

  void *allocate(std::size_t size, std::size_t align);
  void deallocate(void *p) noexcept;
};

// slab allocator for physical pages
// should be used before we have the kernel allocator
template <typename T> class pool_allocator {
  template <typename U> friend class pool_allocator;
  page_pool *m_pool;

public:
  using value_type = T;

  explicit pool_allocator(page_pool &p) noexcept : m_pool(&p) {}
  template <typename U>
  pool_allocator(const pool_allocator<U> &o) noexcept : m_pool(o.m_pool) {}

  T *allocate(std::size_t n) {
    if (n != 1)
      utils::panic("only single object allocations in pool_allocator!!");

    return static_cast<T *>(m_pool->allocate(sizeof(T), alignof(T)));
  }
  void deallocate(T *p, std::size_t) noexcept { m_pool->deallocate(p); }

  template <typename U>
  bool operator==(const pool_allocator<U> &o) const noexcept {
    return m_pool == o.m_pool;
  }
};
} // namespace mm

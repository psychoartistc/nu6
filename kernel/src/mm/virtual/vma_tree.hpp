#pragma once

#include <std/spinlock.hpp>
#include <mm/physical/page_allocator.hpp>
#include <utils/panic.hpp>
#include <mm/common.hpp>
#include <std/dsa/rbtree.hpp>

namespace mm {

// WARNING: this allocator is very very terrible and is highly fragmented
// its not for use anywhere outside:
// temporary stubs where we dont have the kernel allocator yet
// or in the kernel vma tree
template <typename T> class page_bump_allocator {
private:
  static constexpr std::size_t align_of = alignof(T);

  std::uintptr_t m_currPage{0x0};
  std::size_t m_currPageUsed = 0;

  static constexpr std::size_t align_up(std::size_t v) noexcept {
    return (v + (align_of - 1)) & ~(align_of - 1);
  }

  bool ensure_page() noexcept {
    if (m_currPage != 0x0)
      return true;

    auto res = mm::page_allocator::allocate(1);
    if (!res.has_value())
      return false;

    m_currPage = *res;
    m_currPageUsed = 0;
    return true;
  }

  bool bump_page() noexcept {
    auto res = mm::page_allocator::allocate(1);
    if (!res.has_value())
      return false;

    m_currPage = *res;
    m_currPageUsed = 0;
    return true;
  }

public:
  using value_type = T;

  page_bump_allocator() noexcept = default;

  template <typename U>
  page_bump_allocator(const page_bump_allocator<U> &) noexcept {
    auto res = mm::page_allocator::allocate(1);
    assert(res.has_value());
    m_currPage = *res;
    m_currPageUsed = 0;
  }

  T *allocate(std::size_t n) {
    if (n == 0)
      return nullptr;

    if (n > (static_cast<std::size_t>(-1) / sizeof(T)))
      utils::panic("allocation size overflow");

    std::size_t bytes = n * sizeof(T);

    std::size_t aligned_used = align_up(m_currPageUsed);

    if (bytes > PAGE_SIZE)
      utils::panic(" single allocation exceeds page size");

    if (aligned_used + bytes > PAGE_SIZE) {
      if (!ensure_page() || !bump_page())
        utils::panic(" out of physical pages");

      aligned_used = 0;
    }

    if (!ensure_page())
      utils::panic("failed to allocate page");

    T *ptr = reinterpret_cast<T *>(mm::p2v(m_currPage + aligned_used));
    m_currPageUsed = aligned_used + bytes;
    return ptr;
  }

  void deallocate(T *p, std::size_t n) noexcept {
    (void)p;
    (void)n;
  }

  template <typename U>
  bool operator==(const page_bump_allocator<U> &) const noexcept {
    return true;
  }

  template <typename U>
  bool operator!=(const page_bump_allocator<U> &) const noexcept {
    return false;
  }
};

struct vma {
  std::uintptr_t address;
  std::uint64_t count;
};

struct vma_less {
  using is_transparent = void;

  static constexpr std::uintptr_t last(const vma &v) noexcept {
    return v.address + v.count * PAGE_SIZE - 1;
  }
  constexpr bool operator()(const vma &a, const vma &b) const noexcept {
    return last(a) < b.address;
  }
  constexpr bool operator()(const vma &a, std::uintptr_t p) const noexcept {
    return last(a) < p;
  }
  constexpr bool operator()(std::uintptr_t p, const vma &a) const noexcept {
    return p < a.address;
  }
};

// the kernel vma tree must use page_bump_allocator
// userland processes should use the normal allocator
template <class Allocator = std::allocator<vma>> class vma_tree {
private:
  std::rb_tree<vma, std::rb_identity, vma_less, Allocator> m_rbtree{};
  std::uintptr_t m_vasStart{0x0};
  std::uintptr_t m_vasEnd{0x0};
  std::spinlock_mutex m_mtx{};

public:
  vma_tree(std::uintptr_t vasStart, std::uintptr_t vasEnd) {}

  // used in demand paging
  vma &find(std::uintptr_t address) {}

  vma allocate(std::size_t size) {}
};
} // namespace mm

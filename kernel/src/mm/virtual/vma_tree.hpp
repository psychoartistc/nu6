#pragma once

#include <utils/fmt.hpp>
#include <logging/logging.hpp>
#include <std/lock_guard.hpp>
#include <std/spinlock.hpp>
#include <mm/physical/page_allocator.hpp>
#include <utils/panic.hpp>
#include <mm/common.hpp>
#include <std/dsa/rbtree.hpp>

namespace mm {

struct vma {
  std::uintptr_t address{0x0};

  // num of pages
  std::uint64_t count{0};

  // this is not really used by the tree but by virtual_allocate
  bool mapped{false};
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
  logging::named_logger m_logger{"vma_tree"};

public:
  vma_tree(std::uintptr_t vasStart, std::uintptr_t vasEnd)
      : m_vasStart((vasStart + PAGE_SIZE - 1) & ~std::uintptr_t(PAGE_SIZE - 1)),
        m_vasEnd(vasEnd & ~std::uintptr_t(PAGE_SIZE - 1)) {
    std::size_t bytes = m_vasEnd - m_vasStart;
    std::array<char, utils::FMT_SIZE_BUFSIZE> buff{};
    utils::fmt_size(buff, bytes);
    m_logger.info("managing %s (%zu bytes) virtual address space", buff.data(),
                  bytes);
  }

  vma_tree(const vma_tree &) = delete;
  vma_tree &operator=(const vma_tree &) = delete;

  std::uintptr_t address_space_start() const { return m_vasStart; }
  std::uintptr_t address_space_end() const { return m_vasEnd; }

  // used in demand paging
  vma *find(std::uintptr_t address) {
    std::lock_guard<std::spinlock_mutex> lock(m_mtx);
    auto it = m_rbtree.find(address);
    return it == m_rbtree.end() ? nullptr : const_cast<vma *>(&*it);
  }

  vma *allocate(std::size_t size) {
    if (size == 0 || size > ~std::size_t(0) - (PAGE_SIZE - 1))
      return nullptr;

    std::uint64_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    std::lock_guard<std::spinlock_mutex> lock(m_mtx);
    std::uintptr_t cursor = m_vasStart;

    for (auto it = m_rbtree.begin();; ++it) {
      bool at_end = (it == m_rbtree.end());
      std::uintptr_t limit = at_end ? m_vasEnd : it->address;

      if (limit > cursor && (limit - cursor) / PAGE_SIZE >= pages) {
        vma v{cursor, pages};
        auto a = m_rbtree.emplace_hint(it, v);
        return const_cast<vma *>(&(*a));
      }

      if (at_end)
        return nullptr;

      cursor = it->address + it->count * PAGE_SIZE;
    }
  }

  //!!!size in bytes!!!
  std::size_t erase(std::uintptr_t address, std::size_t size) {
    if (size == 0)
      return 0;

    constexpr std::uintptr_t pmask = PAGE_SIZE - 1;
    std::uintptr_t lo = address & ~pmask;

    std::uintptr_t hi = (size - 1 > ~std::uintptr_t(0) - address)
                            ? ~std::uintptr_t(0)
                            : address + (size - 1);
    hi |= pmask;

    vma query{lo, (hi - lo) / PAGE_SIZE + 1};

    std::lock_guard<std::spinlock_mutex> g(m_mtx);
    auto [first, last] = m_rbtree.equal_range(query);
    if (first == last)
      return 0;

    std::size_t touched = static_cast<std::size_t>(std::distance(first, last));
    auto lastIt = std::prev(last);

    vma &f = const_cast<vma &>(*first);
    vma &l = const_cast<vma &>(*lastIt);

    bool hasLeft = f.address < lo;
    bool hasRight = vma_less::last(l) > hi;

    std::uint64_t leftPages = hasLeft ? (lo - f.address) / PAGE_SIZE : 0;
    std::uint64_t rightPages =
        hasRight ? (vma_less::last(l) - hi) / PAGE_SIZE : 0;

    if (first == lastIt) {
      if (hasLeft && hasRight) {
        f.count = leftPages;
        m_rbtree.emplace_hint(last, vma{hi + 1, rightPages});
      } else if (hasLeft) {
        f.count = leftPages;
      } else if (hasRight) {
        f.address = hi + 1;
        f.count = rightPages;
      } else {
        m_rbtree.erase(first);
      }
      return 1;
    }

    auto eraseFrom = first;
    auto eraseTo = last;
    if (hasLeft) {
      f.count = leftPages;
      ++eraseFrom;
    }
    if (hasRight) {
      l.address = hi + 1;
      l.count = rightPages;
      eraseTo = lastIt;
    }
    m_rbtree.erase(eraseFrom, eraseTo);
    return touched;
  }

  bool free(vma v) {
    std::lock_guard<std::spinlock_mutex> s(m_mtx);
    auto it = m_rbtree.find(v.address);

    if (it == m_rbtree.end() || it->address != v.address ||
        it->count != v.count) {

      return false;
    }

    m_rbtree.erase(it);
    return true;
  }
};
} // namespace mm

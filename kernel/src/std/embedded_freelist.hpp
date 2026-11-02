#pragma once

#include <atomic>
#include <cstdint>

namespace std {

class embedded_freelist {
private:
  static inline constexpr std::uint64_t POINTER_MASK = 0x0000FFFFFFFFFFFFULL;
  static inline constexpr std::uint32_t TAG_SHIFT = 48;

private:
  struct node {
    node *next;
  };

  static inline constexpr std::uint64_t pack(node *ptr, std::uint16_t tag) {
    return (static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(ptr)) &
            POINTER_MASK) |
           (static_cast<std::uint64_t>(tag) << TAG_SHIFT);
  }

  static inline constexpr node *get_ptr(std::uint64_t value) {
    auto ptr = value & POINTER_MASK;

    if (ptr & (1ULL << 47))
      ptr |= ~POINTER_MASK;

    return reinterpret_cast<node *>(static_cast<std::uintptr_t>(ptr));
  }

  static std::uint16_t get_tag(std::uint64_t value) {
    return static_cast<std::uint16_t>(value >> TAG_SHIFT);
  }

  std::atomic<std::uint64_t> m_head{0};

public:
  bool is_lock_free() const { return m_head.is_lock_free(); }

  void push(void *memory) {
    if (!memory)
      return;

    auto *n = static_cast<node *>(memory);

    auto old = m_head.load(std::memory_order_relaxed);

    for (;;) {
      n->next = get_ptr(old);

      auto desired = pack(n, get_tag(old) + 1);

      if (m_head.compare_exchange_weak(old, desired, std::memory_order_release,
                                       std::memory_order_relaxed))
        return;
    }
  }

  void *pop() {
    auto old = m_head.load(std::memory_order_acquire);

    for (;;) {
      node *n = get_ptr(old);

      if (!n)
        return nullptr;

      auto desired = pack(n->next, get_tag(old) + 1);

      if (m_head.compare_exchange_weak(old, desired, std::memory_order_acquire,
                                       std::memory_order_relaxed))
        return n;
    }
  }
};

} // namespace std

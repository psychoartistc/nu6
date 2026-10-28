#pragma once

#include <arch/mm/tlb.hpp>
#include <cstddef>
#include <cstdint>

namespace mm {
class page_table;

namespace tlb {
static inline constexpr std::uint64_t SHOOTDOWN_INT_NO = 0xEF;

class flush_batch {
public:
  static constexpr std::size_t FULL_FLUSH_CEILING = 33;
  static constexpr std::size_t MAX_RANGES = 8;

  struct range {
    std::uintptr_t start;
    std::size_t count;
    int level;
  };

  static constexpr std::uintptr_t leaf_size(int level) {
    return static_cast<std::uintptr_t>(1) << (12 + 9 * (level - 1));
  }

  void add(std::uintptr_t virt, int level) {
    if (m_full)
      return;

    const std::uintptr_t size = leaf_size(level);
    virt &= ~(size - 1);

    if (virt >> 63)
      m_kernel = true;

    if (m_n) {
      range &last = m_r[m_n - 1];

      // evil range checks
      if (last.level == level) {
        std::uintptr_t end = last.start + last.count * size;
        if (virt >= last.start && virt < end)
          return;

        if (virt == end) {
          last.count++;
          bump();
          return;
        }
      }
    }

    if (m_n == MAX_RANGES) {
      m_full = true;
      return;
    }

    m_r[m_n++] = {virt, 1, level};
    bump();
  }

  bool empty() const { return m_n == 0 && !m_full; }
  bool full() const { return m_full; }
  bool touches_kernel() const { return m_kernel; }

  const range *begin() const { return m_r; }
  const range *end() const { return m_r + m_n; }

private:
  void bump() {
    if (++m_invlpgs > FULL_FLUSH_CEILING)
      m_full = true;
  }

  range m_r[MAX_RANGES]{};
  std::size_t m_n = 0;
  std::size_t m_invlpgs = 0;
  bool m_full = false;
  bool m_kernel = false;
};

inline void flush_local(const flush_batch &b) {
  if (b.empty())
    return;

  if (b.full()) {
    arch::mm::flush_all();
    return;
  }

  for (const auto &r : b) {
    const std::uintptr_t size = flush_batch::leaf_size(r.level);
    for (std::size_t i = 0; i < r.count; i++)
      arch::mm::invlpg(r.start + i * size);
  }
}

void on_shootdown(page_table &tbl, const flush_batch &batch);
} // namespace tlb
} // namespace mm

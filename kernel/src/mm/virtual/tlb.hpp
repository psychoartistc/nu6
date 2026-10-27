#pragma once

#include <mm/virtual/page_table.hpp>

namespace mm {
namespace tlb {
static inline constexpr std::uint64_t SHOOTDOWN_INT_NO = 0xEF;

void on_shootdown(page_table &tbl, std::uintptr_t virt,
                  std::uint64_t pageCount);
} // namespace tlb
} // namespace mm

#pragma once

#include <mm/physical/page_allocator.hpp>
#include <std/spinlock.hpp>

namespace mm {
class page_table {
public:
  enum class error_code {
    entry_exists,
  };

private:
  std::uintptr_t m_pml4Phys{0x0};
  std::spinlock_mutex m_mtx{};

public:
  page_table();
  ~page_table();
};
} // namespace mm

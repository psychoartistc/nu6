#pragma once

#include <mm/virtual/slub_backend.hpp>

namespace mm {
using user_vma_tree_t = vma_tree<>;

// address space for a userland process
class address_space {
private:
  std::pair<std::uintptr_t, std::uintptr_t> m_bounds{};

  user_vma_tree_t m_vmaTree;
  page_table m_pageTable{};

public:
  address_space();

  std::pair<std::uintptr_t, std::uintptr_t> bounds() const { return m_bounds; }

  user_vma_tree_t &vma_tree();
  page_table &page_table();
};
} // namespace mm

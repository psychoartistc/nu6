#include <mm/address_space.hpp>

namespace mm {

user_vma_tree_t &address_space::vma_tree() { return m_vmaTree; }
page_table &address_space::page_table() { return m_pageTable; }
} // namespace mm

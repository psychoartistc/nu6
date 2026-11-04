#include <mm/address_space.hpp>
#include <mm/virtual/aslr.hpp>

namespace mm {

address_space::address_space()
    : m_bounds(aslr_manager::generate_user()), m_vmaTree(m_bounds) {}

user_vma_tree_t &address_space::vma_tree() { return m_vmaTree; }
page_table &address_space::page_table() { return m_pageTable; }
} // namespace mm

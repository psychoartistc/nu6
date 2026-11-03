#include <mm/virtual/allocator.hpp>

namespace mm {
std::expected<void, page_table::error_code>
map_vma(page_table &pt, vma &vma, protection prot, bool bypass_wxorx) {
  // #ifdef PAGE_ALLOC_ONLY_4KB
  for (std::size_t i = 0; i < vma.count; i++) {
    std::uintptr_t virt = vma.address + i * PAGE_SIZE;
    auto res = page_allocator::allocate(1);
    if (!res)
      return std::unexpected<page_table::error_code>(
          page_table::error_code::no_memory);

    auto res2 =
        pt.map(*res, virt, prot, page_table::page_type::normal, bypass_wxorx);
    if (!res2)
      return std::unexpected<page_table::error_code>(res2.error());
  }
  // #endif

  vma.mapped = true;
  return {};
}
} // namespace mm

#include <mm/virtual/page_table.hpp>
#include <arch/mm/tlb.hpp>
#include <mm/common.hpp>
#include <mm/virtual/tlb.hpp>

namespace mm {
void page_table::tlb_flush(std::uintptr_t virt, std::size_t pages) {
  for (size_t i = 0; i < pages; i++)
    arch::mm::invlpg(virt + i * mm::PAGE_SIZE);

  tlb::on_shootdown(*this, virt, pages);
}
} // namespace mm

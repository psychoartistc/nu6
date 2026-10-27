#include "arch/mm/paging.hpp"
#include <mm/virtual/page_table.hpp>
#include <std/memory.hpp>
#include <utils/panic.hpp>
#include <arch/mm/tlb.hpp>
#include <mm/virtual/tlb.hpp>

namespace mm {
void page_table::tlb_flush(std::uintptr_t virt, std::size_t pages) {
  for (size_t i = 0; i < pages; i++)
    arch::mm::invlpg(virt + i * mm::PAGE_SIZE);

  tlb::on_shootdown(*this, virt, pages);
}

std::expected<std::uintptr_t, page_allocator::error_code>
page_table::alloc_table() {
  auto res = page_allocator::allocate(1);
  if (!res) {
    m_logger.warn("failed to allocate page table! error: %d",
                  static_cast<int>(res.error()));
    return res;
  }

  std::uintptr_t *ptr = reinterpret_cast<std::uintptr_t *>(p2v(*res));
  memset(ptr, 0, mm::PAGE_SIZE);

  return res;
}

std::expected<void, page_table::error_code>
page_table::clone_level(std::uint64_t *dst, const std::uint64_t *src,
                        int level) {
  for (std::size_t i = (level == 4 ? 256 : 0); i < 512; i++) {
    std::uint64_t e = src[i];

    if (!pte_has_flag(e, arch::mm::P_X86_PRESENT)) {
      if (level == 4) {
        auto res = alloc_table();
        if (!res)
          return std::unexpected<error_code>(error_code::no_memory);

        dst[i] = *res | arch::mm::P_X86_PRESENT | arch::mm::P_X86_WRITABLE |
                 arch::mm::P_X86_OWNED;
      }
      continue;
    }

    if (level == 1 || (level < 4 && pte_has_flag(e, arch::mm::P_X86_HUGE))) {
      dst[i] = e;
      continue;
    }

    auto res = alloc_table();
    if (!res)
      return std::unexpected<error_code>(error_code::no_memory);

    auto res2 = clone_level(reinterpret_cast<std::uint64_t *>(p2v(*res)),
                            table_of(e), level - 1);
    if (!res2)
      return res2;

    dst[i] = *res | (e & arch::mm::PT_FLAGS_MASK) | arch::mm::P_X86_OWNED;
  }

  return std::expected<void, page_table::error_code>();
}

page_table::page_table() {
  auto res = alloc_table();
  if (!res)
    utils::panic("error when allocating kernel pml4: %d",
                 static_cast<int>(res.error()));

  m_pml4Phys = *res;
}

void page_table::clone_bootloader_pages() {
  std::uint64_t *src = reinterpret_cast<std::uint64_t *>(
      p2v(arch::mm::read_cr3() & arch::mm::MASK_4K));

  auto res =
      clone_level(reinterpret_cast<std::uint64_t *>(p2v(m_pml4Phys)), src, 4);
  if (!res)
    utils::panic("error when cloning bootloader pages: %d",
                 static_cast<int>(res.error()));
}

void page_table::switch_to() {
  arch::mm::write_cr3(static_cast<std::uint64_t>(m_pml4Phys));
}

} // namespace mm

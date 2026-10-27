#pragma once

#include <arch/mm/nx.hpp>
#include <mm/physical/page_allocator.hpp>
#include <std/spinlock.hpp>
#include <arch/mm/paging.hpp>
#include <mm/virtual/protection.hpp>

namespace mm {
class page_table {
public:
  enum class error_code { entry_exists, wxorx_violation };

private:
  static inline constexpr std::uint64_t protection_to_pte(protection vm) {
    std::uint64_t flags = 0;
    if (protection_has_flag(vm, protection::write))
      flags |= arch::mm::P_X86_WRITABLE;
    if (protection_has_flag(vm, protection::nocache))
      flags |= arch::mm::P_X86_NOCACHE;
    if (arch::mm::nx_bit_supported() &&
        !protection_has_flag(vm, protection::execute))
      flags |= arch::mm::P_X86_NX;
    if (!protection_has_flag(vm, protection::kernel))
      flags |= arch::mm::P_X86_USER;
    return flags;
  }

  static inline protection pte_to_protection(std::uint64_t pte) {
    protection prot = protection::none;
    if (pte_has_flag(pte, arch::mm::P_X86_WRITABLE))
      prot |= protection::write;
    if (pte_has_flag(pte, arch::mm::P_X86_NOCACHE))
      prot |= protection::nocache;
    if (arch::mm::nx_bit_supported() && !pte_has_flag(pte, arch::mm::P_X86_NX))
      prot |= protection::execute;
    if (!pte_has_flag(pte, arch::mm::P_X86_USER))
      prot |= protection::kernel;

    return prot;
  }

  static inline uint64_t leaf_flags(protection protect) {
    return protection_to_pte(protect) | arch::mm::P_X86_PRESENT |
           (protection_has_flag(protect, protection::kernel)
                ? arch::mm::P_X86_GLOBAL
                : 0);
  }

private:
  std::uintptr_t m_pml4Phys{0x0};
  std::spinlock_mutex m_mtx{};

public:
  page_table();
  ~page_table();
};

} // namespace mm

#pragma once

#include <mm/common.hpp>
#include <logging/logging.hpp>
#include <arch/mm/nx.hpp>
#include <mm/physical/page_allocator.hpp>
#include <std/spinlock.hpp>
#include <arch/mm/paging.hpp>
#include <mm/virtual/protection.hpp>

namespace mm {
namespace tlb {
class flush_batch;
}

class page_table {
public:
  enum class error_code {
    entry_exists,
    no_entry,
    wxorx_violation,
    no_memory,
    bad_address,
    unsupported
  };
  enum class page_type { normal, super, mega };

  static inline constexpr std::size_t pages_level(int level) {
    return static_cast<std::size_t>(1) << (9 * (level - 1));
  }

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

  static inline std::size_t page_idx(std::uintptr_t virt, int level) {
    return (virt >> (12 + 9 * (level - 1))) & 0x1FF;
  }

  static inline std::uint64_t *table_of(std::uint64_t entry) {
    return reinterpret_cast<std::uint64_t *>(p2v(entry & arch::mm::MASK_4K));
  }

  static inline constexpr bool page_is_canonical(std::uintptr_t v) {
    std::int64_t s = (std::int64_t)v;
    return ((s << 16) >> 16) == s;
  }

  static inline constexpr std::uint64_t page_level_addr_mask(int level) {
    return level == 1   ? arch::mm::MASK_4K
           : level == 2 ? arch::mm::MASK_2M
                        : arch::mm::MASK_1G;
  }

private:
  std::expected<std::uintptr_t, page_allocator::error_code> alloc_table();

  std::expected<void, error_code>
  clone_level(std::uint64_t *dst, const std::uint64_t *src, int level);

  std::expected<void, error_code> map_at_level(std::uintptr_t phys,
                                               std::uintptr_t virt,
                                               protection prot, int level,
                                               bool bypass_wxorx);

  std::expected<std::uint64_t *, error_code> walk_create(std::uintptr_t virt,
                                                         int level, bool user);

  std::expected<std::pair<std::uint64_t *, int>, error_code>
  find_leaf(std::uintptr_t virt, bool want_user);

  std::expected<void, error_code> split_leaf(std::uint64_t *e, int level);

  bool set_leaf_protect(std::uint64_t *e, int level, protection prot);

  static std::expected<void, error_code> check_range(std::uintptr_t start,
                                                     std::size_t pages);

private:
  logging::named_logger m_logger{"page_table"};
  std::uintptr_t m_pml4Phys{0x0};
  std::spinlock_mutex m_mtx{};

public:
  page_table();
  ~page_table();

  std::expected<void, error_code> map(std::uintptr_t phys, std::uintptr_t virt,
                                      protection prot,
                                      page_type type = page_type::normal,
                                      bool no_wxorx = false);

  std::expected<void, error_code> protect(std::uintptr_t virt, protection prot,
                                          std::size_t pages = 1,
                                          bool no_wxorx = false);

  std::expected<std::uintptr_t, error_code> physical_of(std::uintptr_t addr);

  std::uintptr_t pml4_phys() const { return m_pml4Phys; }

  void switch_to();

  std::expected<void, error_code> unmap(std::uintptr_t virt,
                                        std::size_t pages = 1);
  void tlb_flush(const tlb::flush_batch &batch);

  void clone_bootloader_pages();
};

} // namespace mm

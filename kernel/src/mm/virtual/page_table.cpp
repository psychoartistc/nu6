#include "arch/mm/paging.hpp"
#include <std/spinlock.hpp>
#include <std/lock_guard.hpp>
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
  // start from entry 256 on level 4
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

    // recursion, shouldnt be too harmful
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

static inline size_t pages_chunk(uintptr_t virt, int level, size_t rem) {
  size_t span = page_table::pages_level(level);
  size_t off = (virt >> 12) & (span - 1);
  size_t n = span - off;
  return n < rem ? n : rem;
}

std::expected<std::uint64_t *, page_table::error_code>
page_table::walk_create(std::uintptr_t virt, int level, bool user) {
  std::uint64_t *table = reinterpret_cast<std::uint64_t *>(p2v(m_pml4Phys));

  for (int l = 4; l > level; l--) {
    std::uint64_t *entry = &table[page_idx(virt, l)];
    std::atomic_ref<std::uint64_t> e(*entry);

    std::uint64_t v = *entry;

    if (!pte_has_flag(v, arch::mm::P_X86_PRESENT)) {
      auto res = alloc_table();
      if (!res)
        return std::unexpected<error_code>(error_code::no_memory);

      v = *res | arch::mm::P_X86_PRESENT | arch::mm::P_X86_WRITABLE |
          arch::mm::P_X86_OWNED | (user ? arch::mm::P_X86_USER : 0);

      e.store(v, std::memory_order_release);
    } else if (user && !pte_has_flag(v, arch::mm::P_X86_USER)) {
      e.fetch_or(arch::mm::P_X86_USER, std::memory_order_release);
    }
    table = table_of(v);
  }

  return &table[page_idx(virt, level)];
}

std::expected<std::pair<std::uint64_t *, int>, page_table::error_code>
page_table::find_leaf(std::uintptr_t virt, bool want_user) {
  std::uint64_t *table = reinterpret_cast<std::uint64_t *>(p2v(m_pml4Phys));

  for (int level = 4; level >= 1; level--) {
    std::uint64_t *e = &table[page_idx(virt, level)];
    std::atomic_ref<std::uint64_t> ref(*e);

    std::uint64_t v = *e;

    if (!pte_has_flag(v, arch::mm::P_X86_PRESENT))
      return std::unexpected<error_code>(error_code::no_entry);

    if (level == 1 || (level < 4 && pte_has_flag(v, arch::mm::P_X86_HUGE)))
      return std::make_pair(e, level);

    if (want_user && !pte_has_flag(v, arch::mm::P_X86_USER))
      ref.fetch_or(arch::mm::P_X86_USER, std::memory_order_release);

    table = table_of(v);
  }

  return std::unexpected<error_code>(error_code::no_entry);
}

std::expected<void, page_table::error_code>
page_table::map_at_level(std::uintptr_t virt, std::uintptr_t phys,
                         protection prot, int level, bool bypass_wxorx) {
  std::uint64_t size = level == 1   ? PAGE_SIZE
                       : level == 2 ? SUPERPAGE_SIZE
                                    : MEGAPAGE_SIZE;

  if ((virt & (size - 1)) || (phys & (size - 1)))
    return std::unexpected<error_code>(error_code::bad_address);

  // unaligned addresses
  if (!page_is_canonical(virt) || (phys & ~arch::mm::MASK_4K)) {
    // pr_error("unaligned, canonical: %d", page_is_canonical(virt));
    return std::unexpected<error_code>(error_code::bad_address);
  }

  // check for wxorx
  if (!bypass_wxorx && protection_has_flag(prot, protection::write) &&
      protection_has_flag(prot, protection::execute))
    return std::unexpected<error_code>(error_code::wxorx_violation);

  bool user = !protection_has_flag(prot, protection::kernel);

  std::lock_guard<std::spinlock_mutex> lock(m_mtx);

  auto res = walk_create(virt, level, user);
  if (!res)
    return std::unexpected<error_code>(res.error());

  if (pte_has_flag(**res, arch::mm::P_X86_PRESENT))
    return std::unexpected<error_code>(error_code::entry_exists);

  std::atomic_ref<std::uint64_t> entry(**res);
  std::uint64_t v =
      phys | leaf_flags(prot) | (level > 1 ? arch::mm::P_X86_HUGE : 0);

  entry.store(v, std::memory_order_release);

  return std::expected<void, error_code>();
}

static inline constexpr int page_type_to_level(page_table::page_type type) {
  using ty = page_table::page_type;
  switch (type) {
  case ty::normal:
    return 1;
  case ty::super:
    return 2;
  case ty::mega:
    return 3;
  }
}

std::expected<void, page_table::error_code>
page_table::map(std::uintptr_t phys, std::uintptr_t virt, protection prot,
                page_type type, bool no_wxorx) {
  if (type == page_type::mega && !arch::mm::megapages_supported())
    return std::unexpected<error_code>(error_code::unsupported);

  return map_at_level(virt, phys, prot, page_type_to_level(type), no_wxorx);
}

std::expected<void, page_table::error_code>
page_table::unmap(std::uintptr_t virt) {
  if (!page_is_canonical(virt))
    return std::unexpected<error_code>(error_code::bad_address);

  std::lock_guard<std::spinlock_mutex> lock(m_mtx);

  auto res = find_leaf(virt, false);
  if (!res)
    return std::unexpected<error_code>(res.error());

  if (virt & ((pages_level(res->second) * PAGE_SIZE) - 1))
    return std::unexpected<error_code>(error_code::bad_address);

  std::atomic_ref<std::uint64_t> entry(*res->first);

  entry.store(0, std::memory_order_release);

  arch::mm::invlpg(virt);
  tlb_flush(virt, 1);

  return std::expected<void, page_table::error_code>();
}

std::expected<void, page_table::error_code>
page_table::split_leaf(std::uint64_t *e, int level) {
  assert(level == 2 || level == 3 && "attempted to split a 4k page");

  std::uint64_t old = *e;
  auto res = alloc_table();
  if (!res)
    return std::unexpected<error_code>(error_code::no_memory);

  std::uint64_t *t = reinterpret_cast<std::uint64_t *>(p2v(*res));
  std::uint64_t mask = page_level_addr_mask(level);
  std::uint64_t base = old & mask;
  std::uint64_t step = level == 3 ? SUPERPAGE_SIZE : PAGE_SIZE;
  std::uint64_t flags = old & ~mask;

  if (level == 2)
    flags &= ~(arch::mm::P_X86_HUGE | arch::mm::PTE_PAT_HUGE);

  for (size_t i = 0; i < 512; i++)
    t[i] = (base + i * step) | flags;

  std::atomic_ref<std::uint64_t> entry(*e);
  entry.store(*res | arch::mm::P_X86_PRESENT | arch::mm::P_X86_WRITABLE |
                  arch::mm::P_X86_OWNED | (old & arch::mm::P_X86_USER),
              std::memory_order_release);

  return std::expected<void, error_code>();
}

bool page_table::set_leaf_protect(std::uint64_t *e, int level,
                                  protection prot) {
  std::atomic_ref<std::uint64_t> ref(*e);

  std::uint64_t keep = page_level_addr_mask(level) | arch::mm::PTE_ACCESSED |
                       arch::mm::PTE_DIRTY | arch::mm::P_X86_OWNED;

  if (level > 1)
    keep |= arch::mm::P_X86_HUGE | arch::mm::PTE_PAT_HUGE;

  std::uint64_t nf = leaf_flags(prot);
  std::uint64_t old = ref.load(std::memory_order_relaxed);
  std::uint64_t nw{};

  do {
    nw = (old & keep) | nf;
    if (nw == old)
      return false;
  } while (!ref.compare_exchange_strong(old, nw, std::memory_order_acq_rel,
                                        std::memory_order_relaxed));

  return true;
}

std::expected<void, page_table::error_code>
page_table::protect(std::uintptr_t start, protection prot, std::size_t pages,
                    bool no_wxorx) {
  if (!no_wxorx && protection_has_flag(prot, protection::write) &&
      protection_has_flag(prot, protection::execute))
    return std::unexpected<error_code>(error_code::wxorx_violation);

  if (pages == 0)
    return std::expected<void, error_code>();

  if (start & (PAGE_SIZE - 1))
    return std::unexpected<error_code>(error_code::bad_address);

  if (pages > (std::numeric_limits<std::uintptr_t>::max() / PAGE_SIZE) ||
      start + pages * PAGE_SIZE < start)
    return std::unexpected<error_code>(error_code::bad_address);

  if (!page_is_canonical(start) ||
      !page_is_canonical(start + pages * PAGE_SIZE - 1))
    return std::unexpected<error_code>(error_code::bad_address);

  bool user = !protection_has_flag(prot, protection::kernel);
  std::lock_guard<std::spinlock_mutex> lock(m_mtx);

  {
    std::uintptr_t virt = start;
    std::size_t rem = pages;
    while (rem) {
      auto res = find_leaf(virt, false);
      if (!res)
        return std::unexpected<error_code>(res.error());

      std::size_t n = pages_chunk(virt, res->second, rem);
      virt += n * PAGE_SIZE;
      rem -= n;
    }
  }

  tlb::flush_batch batch = {.start = 0, .pages = 0};
  std::uintptr_t virt = start;
  std::size_t rem = pages;

  std::optional<error_code> ec = std::nullopt;
  while (rem) {
    auto res = find_leaf(virt, user);
    if (!res)
      return std::unexpected<error_code>(res.error());

    std::size_t n = pages_chunk(virt, res->second, rem);
    if (res->second > 1 && n < pages_level(res->second)) {
      auto res2 = split_leaf(res->first, res->second);
      if (!res2) {
        ec = res2.error();
        break;
      }
      continue;
    }

    if (set_leaf_protect(res->first, res->second, prot)) {
      if (n == 0)
        batch.start = virt;
      batch.pages++;
    }

    virt += n * PAGE_SIZE;
    rem -= n;
  }

  tlb_flush(batch.start, batch.pages);

  if (!ec)
    return std::expected<void, error_code>();

  return std::unexpected<error_code>(*ec);
}

std::expected<std::uintptr_t, page_table::error_code>
page_table::physical_of(std::uintptr_t addr) {

  std::uint64_t e{0};
  int level{};
  {
    std::lock_guard<std::spinlock_mutex> lock(m_mtx);

    auto res = find_leaf(addr, false);
    if (!res)
      return std::unexpected<error_code>(res.error());

    e = *res->first;
    level = res->second;
  }

  if (!e)
    return 0;

  uint64_t off_mask = (pages_level(level) * PAGE_SIZE) - 1;
  return (e & page_level_addr_mask(level)) | (addr & off_mask);
}

} // namespace mm

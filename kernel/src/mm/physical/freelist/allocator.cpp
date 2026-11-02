#include <mm/physical/freelist/freelist.hpp>

#ifdef PAGE_ALLOC_USE_FREELIST
namespace mm {
static freelist_allocator s_freelist;

bool page_allocator::initialize() {
  return s_freelist.reclaim_type(LIMINE_MEMMAP_USABLE);
}

std::expected<std::uintptr_t, page_allocator::error_code>
page_allocator::allocate(std::uint64_t count) {
  if (count > 1)
    return std::unexpected<error_code>(error_code::unimplemented);

  return s_freelist.pop();
}

bool page_allocator::free(std::uintptr_t at, std::uint64_t count) {
  if (count > 1 || count == 0)
    return false;

  return s_freelist.push(at);
}

bool page_allocator::reclaim_acpi() {
  return s_freelist.reclaim_type(LIMINE_MEMMAP_ACPI_RECLAIMABLE);
}

bool page_allocator::reclaim_bootloader() {
  return s_freelist.reclaim_type(LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE);
}

} // namespace mm

#endif

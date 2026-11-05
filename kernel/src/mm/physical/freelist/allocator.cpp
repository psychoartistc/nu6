#include <mm/physical/freelist/freelist.hpp>

#ifdef PAGE_ALLOC_USE_FREELIST
namespace mm {
static freelist_allocator s_freelist;

bool page_allocator::initialize() {
  bool res = s_freelist.reclaim_type(LIMINE_MEMMAP_USABLE);
  if (res)
    s_logger.info("freelist initialized");

  return res;
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
  bool res = s_freelist.reclaim_type(LIMINE_MEMMAP_ACPI_RECLAIMABLE);
  if (res)
    s_logger.info("acpi regions reclaimed");

  return res;
}

bool page_allocator::reclaim_bootloader() {
  bool res = s_freelist.reclaim_type(LIMINE_MEMMAP_ACPI_RECLAIMABLE);
  if (res)
    s_logger.info("bootloader page tables reclaimed");

  return res;
}

} // namespace mm

#endif

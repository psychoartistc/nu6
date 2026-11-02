#include <mm/physical/page_allocator.hpp>
#include <utils/fmt.hpp>
#include <mm/common.hpp>
#include <limine/requests.hpp>
#include <utils/math.hpp>
#include <logging/logging.hpp>
#include <std/embedded_freelist.hpp>

static std::embedded_freelist s_freelist{};
static std::atomic<std::uint64_t> s_totalFrames{0};
static std::atomic<std::uint64_t> s_freeFrames{0};
static logging::named_logger s_logger{"freelist"};

namespace mm {
static bool reclaim_type(std::uint64_t type, bool updateTotalFrames) {
  auto *resp = limine::memmap_request.response;
  if (!resp)
    return false;

  for (std::size_t i = 0; i < resp->entry_count; i++) {
    auto *entry = resp->entries[i];
    if (!entry || entry->type != type)
      continue;

    std::size_t pages = utils::div_floor<std::size_t>(entry->length, PAGE_SIZE);

    for (std::size_t j = 0; j < pages; j++) {
      std::uintptr_t pageStart = entry->base + j * PAGE_SIZE;

      s_freelist.push(p2v(pageStart));
      if (updateTotalFrames)
        s_totalFrames.fetch_add(1);

      s_freeFrames.fetch_add(1, std::memory_order_release);
    }
  }

  std::array<char, utils::FMT_SIZE_BUFSIZE> buf{};
  std::uint64_t freeFrames = s_freeFrames.load(std::memory_order_relaxed);
  utils::fmt_size(buf, freeFrames * PAGE_SIZE);

  s_logger.info(
      "successfully reclaimed %zu pages (%s physical memory) of type %zu",
      freeFrames, buf.data(), type);

  return true;
}

#ifdef PAGE_ALLOC_USE_FREELIST

bool page_allocator::initialize() {
  return reclaim_type(LIMINE_MEMMAP_USABLE, true);
}

std::expected<std::uintptr_t, page_allocator::error_code>
page_allocator::allocate(std::uint64_t count) {
  if (count > 1)
    return std::unexpected<page_allocator::error_code>(
        error_code::unimplemented);

  if (count == 0)
    return std::unexpected<page_allocator::error_code>(error_code::bad_arg);

  if (s_freeFrames.load(std::memory_order_acquire) == 0)
    return std::unexpected<page_allocator::error_code>(error_code::no_mem);

  s_freeFrames.fetch_sub(1, std::memory_order_release);
  return v2p(s_freelist.pop());
}

bool page_allocator::free(std::uintptr_t at, std::uint64_t count) {
  if (count > 1 || count == 0)
    return false;

  s_freelist.push(p2v(at));
  s_freeFrames.fetch_add(1, std::memory_order_release);
  return true;
}

bool page_allocator::reclaim_acpi() {
  return reclaim_type(LIMINE_MEMMAP_ACPI_RECLAIMABLE, true);
}
bool page_allocator::reclaim_bootloader() {
  return reclaim_type(LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE, true);
}
} // namespace mm

#endif

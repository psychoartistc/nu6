// #include <mm/physical/page_allocator.hpp>
#include <utils/fmt.hpp>
#include <mm/common.hpp>
#include <mm/physical/freelist/freelist.hpp>
#include <limine/requests.hpp>
#include <utils/math.hpp>

namespace mm {
bool freelist_allocator::reclaim_type(std::uint64_t type,
                                      bool update_total_frames) {
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

      push(pageStart);
      if (update_total_frames)
        m_totalFrames.fetch_add(1);
    }
  }

  std::array<char, utils::FMT_SIZE_BUFSIZE> buf{};
  utils::fmt_size(buf, free_frames() * PAGE_SIZE);

  m_logger.info(
      "successfully reclaimed %zu pages (%s physical memory) of type %zu",
      free_frames(), buf.data(), type);

  return true;
}

std::expected<std::uintptr_t, mm::page_allocator::error_code>
freelist_allocator::pop() {
  if (!m_bNotEmpty.load())
    return std::unexpected<page_allocator::error_code>(
        page_allocator::error_code::no_mem);
  std::uintptr_t head = 0x0;
  {
    std::lock_guard<std::spinlock_mutex> lock(m_mutex);

    head = m_head;
    std::uintptr_t *headVirt = static_cast<std::uintptr_t *>(p2v(head));

    m_head = *headVirt;
  }

  m_freeFrames.fetch_sub(1, std::memory_order_relaxed);
  return head;
}

bool freelist_allocator::push(std::uintptr_t page) {
  if (!utils::page_aligned(page))
    return false;

  {
    std::lock_guard<std::spinlock_mutex> lock(m_mutex);
    std::uintptr_t head = m_head;
    std::uintptr_t *pageVirt = static_cast<std::uintptr_t *>(p2v(page));

    *pageVirt = head;
    m_head = page;
  }
  m_freeFrames.fetch_add(1, std::memory_order_relaxed);
  m_bNotEmpty.store(true);

  return true;
}

#ifdef PAGE_ALLOC_USE_FREELIST

static freelist_allocator s_freelist{};

bool page_allocator::initialize() {
  return s_freelist.reclaim_type(LIMINE_MEMMAP_USABLE, true);
}

std::expected<std::uintptr_t, page_allocator::error_code>
page_allocator::allocate(std::uint64_t count) {
  if (count > 1)
    return std::unexpected<page_allocator::error_code>(
        error_code::unimplemented);

  if (count == 0)
    return std::unexpected<page_allocator::error_code>(error_code::bad_arg);

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

#include <mm/physical/freelist/freelist.hpp>

namespace mm {
bool freelist_allocator::reclaim_type(std::uint64_t type) {
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

      m_freelist.push(p2v(pageStart));

      m_totalFrames.fetch_add(1);

      m_freeFrames.fetch_add(1, std::memory_order_release);
    }
  }

  std::array<char, utils::FMT_SIZE_BUFSIZE> buf{};
  std::uint64_t freeFrames = m_freeFrames.load(std::memory_order_relaxed);
  utils::fmt_size(buf, freeFrames * PAGE_SIZE);

  return true;
}

std::expected<std::uintptr_t, page_allocator::error_code>
freelist_allocator::pop() {
  if (free_frames() == 0)
    return std::unexpected<error_code>(error_code::no_mem);

  void *virt = m_freelist.pop();
  m_freeFrames.fetch_sub(1, std::memory_order_relaxed);

  return v2p(virt);
}

bool freelist_allocator::push(std::uintptr_t a) {
  if (!utils::page_aligned(a))
    return false;

  m_freelist.push(p2v(a));
  m_freeFrames.fetch_add(1, std::memory_order_relaxed);

  return true;
}
} // namespace mm

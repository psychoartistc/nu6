#include <mm/physical/phys_allocator.hpp>

namespace mm {
void page_pool::partial_push(page_header *pg) noexcept {
  pg->part_prev = nullptr;
  pg->part_next = m_partial;

  if (m_partial)
    m_partial->part_prev = pg;

  m_partial = pg;
  pg->in_partial = true;
}
void page_pool::partial_remove(page_header *pg) noexcept {
  if (pg->part_prev)
    pg->part_prev->part_next = pg->part_next;

  else
    m_partial = pg->part_next;

  if (pg->part_next)
    pg->part_next->part_prev = pg->part_prev;

  pg->in_partial = false;
}

page_pool::page_header *page_pool::new_page() {
  auto res = mm::page_allocator::allocate(1);
  if (!res.has_value())
    utils::panic("out of physical pages");

  auto *pg = reinterpret_cast<page_header *>(mm::p2v(*res));
  assert((reinterpret_cast<std::uintptr_t>(pg) & (PAGE_SIZE - 1)) == 0);
  *pg = page_header{};

  pg->phys = *res;
  pg->all_next = m_all;
  if (m_all)
    m_all->all_prev = pg;

  m_all = pg;
  partial_push(pg);
  return pg;
}

void page_pool::release_page(page_pool::page_header *pg) noexcept {
  if (pg->in_partial)
    partial_remove(pg);

  if (pg->all_prev)
    pg->all_prev->all_next = pg->all_next;
  else
    m_all = pg->all_next;

  if (pg->all_next)
    pg->all_next->all_prev = pg->all_prev;

  mm::page_allocator::free(pg->phys, 1);
}

page_pool::~page_pool() {
  for (page_header *pg = m_all; pg;) {
    page_header *next = pg->all_next;
    mm::page_allocator::free(pg->phys, 1);
    pg = next;
  }
}

void page_pool::bind(std::size_t size, std::size_t align) {
  if (m_slotSize != 0) {
    assert(size <= m_slotSize && align <= m_slotAlign);
    return;
  }

  m_slotAlign = align > alignof(void *) ? align : alignof(void *);
  m_slotSize = utils::align_up(size > sizeof(void *) ? size : sizeof(void *),
                               m_slotAlign);

  m_firstSlot = utils::align_up(sizeof(page_header), m_slotAlign);

  if (m_firstSlot + m_slotSize > PAGE_SIZE)
    utils::panic("object does not fit in a page");

  m_capacity = (PAGE_SIZE - m_firstSlot) / m_slotSize;
}

void *page_pool::allocate(std::size_t size, std::size_t align) {
  bind(size, align);
  page_header *pg = m_partial ? m_partial : new_page();
  if (pg->used == 0 && pg->bump == 0 && !pg->free_list) {
  } else if (pg->used == 0) {
    --m_empty;
  }
  void *s;
  if (pg->free_list) {
    s = pg->free_list;
    pg->free_list = *static_cast<void **>(s);
  } else {
    s = slot_at(pg, pg->bump++);
  }
  ++pg->used;
  if (!pg->free_list && pg->bump == m_capacity)
    partial_remove(pg);
  return s;
}

void page_pool::deallocate(void *p) noexcept {
  auto *pg = reinterpret_cast<page_header *>(
      reinterpret_cast<std::uintptr_t>(p) & ~std::uintptr_t(PAGE_SIZE - 1));
  *static_cast<void **>(p) = pg->free_list;
  pg->free_list = p;
  --pg->used;
  if (!pg->in_partial)
    partial_push(pg);

  if (pg->used == 0) {
    if (m_empty >= 1)
      release_page(pg);
    else
      ++m_empty;
  }
}
} // namespace mm

#include <mm/stl.hpp>

void *operator new(std::size_t size) {
  static mm::slub::slub_allocator &instance = mm::slub::slub_allocator::get();
  void *p = instance.allocate(size);
  if (!p) {
    utils::panic("allocation failed, new() tried to allocate %zu bytes", size);

    unreachable();
    return nullptr;
  }

  return p;
}

void *operator new[](std::size_t size) {
  static mm::slub_allocator &instance = mm::slub_allocator::get();
  void *p = instance.allocate(size);
  if (!p) {
    lib::panic("allocation failed, new() tried to allocate %zu bytes", size);

    unreachable();
    return nullptr;
  }

  return p;
}

void operator delete[](void *p) noexcept {
  static mm::slub_allocator &instance = mm::slub_allocator::get();
  instance.free(p);
}

void operator delete(void *p) noexcept {
  static mm::slub_allocator &instance = mm::slub_allocator::get();
  instance.free(p);
}

void operator delete(void *ptr, unsigned long size) noexcept {
  static mm::slub_allocator &instance = mm::slub_allocator::get();
  instance.free(ptr);
}

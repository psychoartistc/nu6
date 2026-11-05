#include <mm/stl.hpp>
#include <mm/virtual/slub_backend.hpp>

void *operator new(std::size_t size) {
  static mm::slub_alloc_t &instance = mm::slub_alloc_t::get();
  void *p = instance.allocate(size);

  return p;
}

void *operator new[](std::size_t size) {
  static mm::slub_alloc_t &instance = mm::slub_alloc_t::get();
  void *p = instance.allocate(size);

  return p;
}

void operator delete[](void *p) noexcept {
  static mm::slub_alloc_t &instance = mm::slub_alloc_t::get();
  instance.free(p);
}

void operator delete(void *p) noexcept {
  static mm::slub_alloc_t &instance = mm::slub_alloc_t::get();
  instance.free(p);
}

void operator delete(void *ptr, unsigned long size) noexcept {
  (void)size;
  static mm::slub_alloc_t &instance = mm::slub_alloc_t::get();
  instance.free(ptr);
}

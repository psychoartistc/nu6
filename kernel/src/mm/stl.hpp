#pragma once

#include <mm/slub.hpp>
#include <utils/panic.hpp>

void *operator new(std::size_t size);
void *operator new[](std::size_t size);
void operator delete(void *ptr, unsigned long size) noexcept;
void operator delete[](void *p) noexcept;
void operator delete(void *p) noexcept;

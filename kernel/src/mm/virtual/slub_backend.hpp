#pragma once

#include <mm/slub.hpp>
#include <mm/virtual/allocator.hpp>

namespace mm {
class vmem_slub_backend {
public:
  void *allocate();
};
} // namespace mm

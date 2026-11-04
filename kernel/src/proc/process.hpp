#pragma once

#include <optional>
#include <mm/address_space.hpp>

namespace proc {
class process {
private:
  // static process &s_current;

private:
  mm::address_space m_addressSpace;

public:
  const mm::address_space &address_space();
};
} // namespace proc

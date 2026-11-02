#pragma once

#include <optional>
#include <mm/address_space.hpp>

namespace proc {
class process {
private:
  mm::address_space m_addressSpace;

public:
  static std::optional<process &> current();
};
} // namespace proc

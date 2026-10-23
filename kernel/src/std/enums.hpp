#pragma once

#include <string_view>

namespace std {

template <auto V>
concept is_enum_val = std::is_enum_v<decltype(V)>;

template <auto V>
constexpr std::string_view enum_name()
  requires is_enum_val<V>
{
  std::string_view name = __PRETTY_FUNCTION__;

  std::size_t start = name.find("V = ") + 4;
  std::size_t end = name.size() - 1;
  std::string_view sub{name.data() + start, end - start};

  if (auto pos = sub.find(';'); pos != std::string_view::npos) {
    sub = sub.substr(0, pos);
  }

  return sub;
}
} // namespace std

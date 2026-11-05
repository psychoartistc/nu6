#pragma once

#include <type_traits>
#include <memory>

namespace std {
namespace detail {
template <class> constexpr bool is_unbounded_array_v = false;
template <class T> constexpr bool is_unbounded_array_v<T[]> = true;

template <class> constexpr bool is_bounded_array_v = false;
template <class T, std::size_t N>
constexpr bool is_bounded_array_v<T[N]> = true;
} // namespace detail

template <class T, class... Args>
std::enable_if_t<!std::is_array<T>::value, std::unique_ptr<T>>
make_unique(Args &&...args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
std::enable_if_t<detail::is_unbounded_array_v<T>, std::unique_ptr<T>>
make_unique(std::size_t n) {
  return std::unique_ptr<T>(new std::remove_extent_t<T>[n]());
}

template <class T, class... Args>
std::enable_if_t<detail::is_bounded_array_v<T>>
make_unique(Args &&...) = delete;
} // namespace std

#pragma once

#include <functional>
#include <string_view>

namespace service {

enum class service_init_order { urgent = 0, after_mem = 1, normal = 2 };

inline constexpr std::string_view SVCLICENSE_GPL = "GPL";
inline constexpr std::string_view SVCLICENSE_GPL2 = "GPL2";
inline constexpr std::string_view SVCLICENSE_GPL3 = "GPL3";

inline constexpr std::string_view SVCLICENSE_BSD0 = "BSD0";
inline constexpr std::string_view SVCLICENSE_BSD2 = "BSD2";
inline constexpr std::string_view SVCLICENSE_BSD3 = "BSD3";
inline constexpr std::string_view SVCLICENSE_BSD4 = "BSD4";

inline constexpr std::string_view SVCLICENSE_APACHE = "Apache";

inline constexpr std::string_view SVCLICENSE_MIT = "MIT";

inline constexpr std::string_view SVCLICENSE_MPL = "MPL";

inline constexpr std::string_view SVCLICENSE_CC0 = "CC0";

inline constexpr std::string_view SVCLICENSE_PROPRIETARY = "Proprietary";

inline constexpr std::string_view SVCLICENSE_DEFAULT = SVCLICENSE_GPL3;

struct service {
  std::function<void()>
};
} // namespace service

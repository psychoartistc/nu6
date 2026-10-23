#pragma once

#include <functional>
#include <string_view>

namespace module {

enum class module_init_order { urgent = 0, after_mem = 1, normal = 2 };

inline constexpr std::string_view MDLLICENSE_GPL = "GPL";
inline constexpr std::string_view MDLLICENSE_GPL2 = "GPL2";
inline constexpr std::string_view MDLLICENSE_GPL3 = "GPL3";

inline constexpr std::string_view MDLLICENSE_BSD0 = "BSD0";
inline constexpr std::string_view MDLLICENSE_BSD2 = "BSD2";
inline constexpr std::string_view MDLLICENSE_BSD3 = "BSD3";
inline constexpr std::string_view MDLLICENSE_BSD4 = "BSD4";

inline constexpr std::string_view MDLLICENSE_APACHE = "Apache";

inline constexpr std::string_view MDLLICENSE_MIT = "MIT";

inline constexpr std::string_view MDLLICENSE_MPL = "MPL";

inline constexpr std::string_view MDLLICENSE_CC0 = "CC0";

inline constexpr std::string_view MDLLICENSE_PROPRIETARY = "Proprietary";

inline constexpr std::string_view MDLLICENSE_DEFAULT = MDLLICENSE_GPL3;

struct module {
  using init_fn_t = bool (*)(void);
  using exit_fn_t = void (*)(void);

  std::string_view name = "???", description = "???", version = "1.0.0",
                   license = MDLLICENSE_DEFAULT;

  init_fn_t init = nullptr;
  exit_fn_t exit = nullptr;

  module_init_order order = module_init_order::normal;
  bool initialized = false;
};

void initialize_some(module_init_order order);
} // namespace module

#define MODULE_SECTION

#define REGISTER_MODULE(symbol, module_name, module_description,               \
                        module_version, module_license, module_order,          \
                        init_function, exit_function)                          \
  [[gnu::section(".modules"), gnu::used,                                       \
    gnu::aligned(8)]] static module::module symbol = {                         \
      .name = module_name,                                                     \
      .description = module_description,                                       \
      .version = module_version,                                               \
      .license = module_license,                                               \
      .init = init_function,                                                   \
      .exit = exit_function,                                                   \
      .order = module_order,                                                   \
      .initialized = false}

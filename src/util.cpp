// msh-to-exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2020 Weston Ortiz
//
// See the LICENSE file for license information.

#include "util.hpp"
#include <fmt/format.h>
#ifdef NO_SOURCE_LOCATION
void msh2exo::failure_check_func(
    bool status, std::string_view msg,
    size_t line, std::string file) {
  if (!status) {
    std::string errmsg =
        fmt::format("Error: {} at {}:{}", msg, file,
                    line);
    throw std::runtime_error(errmsg);
  }
}

#else 
void msh2exo::failure_check_func(
    bool status, std::string_view msg,
    const std::experimental::source_location &location) {
  if (!status) {
    std::string errmsg =
        fmt::format("Error: {} at {}:{} in {}", msg, location.file_name(),
                    location.line(), location.function_name());
    throw std::runtime_error(errmsg);
  }
}
#endif

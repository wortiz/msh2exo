// msh2exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2020 Weston Ortiz
//
// See the LICENSE file for license information.

#pragma once

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <string>

#include "config.hpp"
#ifndef ENABLE_SOURCE_LOCATION

#define MSH2EXO_CHECK(status, msg)                                             \
  msh2exo::failure_check_func(status, msg, __LINE__, __FILE__)

namespace msh2exo {
void failure_check_func(bool status, const std::string &msg, size_t line,
                        std::string file);
}
#else
#include <experimental/source_location>

#define MSH2EXO_CHECK msh2exo::failure_check_func

namespace msh2exo {

void failure_check_func(bool status, const std::string &msg,
                        const std::experimental::source_location &location =
                            std::experimental::source_location::current());
}
#endif

namespace msh2exo {
void print_info_and_exit(void);

template <typename S, typename... Args>
void print_if(bool flag, const S &format, Args &&...args) {
  if (flag) {
    fmt::vprint(format, fmt::make_args_checked<Args...>(format, args...));
  }
}

} // namespace msh2exo

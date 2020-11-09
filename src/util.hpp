// msh-to-exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2020 Weston Ortiz
//
// See the LICENSE file for license information.

#pragma once

#include <string>
#ifdef NO_SOURCE_LOCATION

#define MSH2EXO_CHECK(status, msg) msh2exo::failure_check_func(status, msg, __LINE__, __FILE__)

namespace msh2exo {
void failure_check_func(bool status, std::string_view msg, size_t line, std::string file);
}
#else
#include <experimental/source_location>

#define MSH2EXO_CHECK msh2exo::failure_check_func

namespace msh2exo {

void failure_check_func(bool status, std::string_view msg,
                   const std::experimental::source_location &location =
                       std::experimental::source_location::current());
}
#endif

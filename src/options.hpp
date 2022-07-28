// msh2exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2020 Weston Ortiz
//
// See the LICENSE file for license information.

#pragma once
#include "CLI/App.hpp"

namespace msh2exo {
struct Options {
  std::string input_file;
  std::string output_file;
  bool force = false;
  bool builtin = false;
  bool verbose = false;
  bool version = false;
};

void setup_options(CLI::App &app, Options &options);

void run_msh2exo(Options &options);

} // namespace msh2exo

// msh-to-exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2020 Weston Ortiz
//
// See the LICENSE file for license information.

#include "exodus_writer.hpp"
#include "gmsh_reader.hpp"
#include "options.hpp"

int main(int argc, char **argv) {

  CLI::App app{"msh2exo: mesh conversion utility"};

  msh2exo::Options options = msh2exo::setup_options(app);

  CLI11_PARSE(app, argc, argv);

  msh2exo::run_conversion(options);

  return 0;
}
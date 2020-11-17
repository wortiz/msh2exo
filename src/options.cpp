// msh2exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2020 Weston Ortiz
//
// See the LICENSE file for license information.

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include "config.hpp"
#include "exodus_writer.hpp"
#include "gmsh_reader.hpp"
#include "options.hpp"
#include "util.hpp"

msh2exo::Options msh2exo::setup_options(CLI::App &app) {
  msh2exo::Options options;

  app.add_flag_function(
      "-V,--version", [](auto) { msh2exo::print_info_and_exit(); },
      "print version and basic info");

  app.add_option("input_file", options.input_file, "Input (Gmsh msh) mesh file")
      ->required()
      ->check(CLI::ExistingFile);

  app.add_option("output_file", options.output_file,
                 "Output (ExodusII) mesh file")
      ->required();
  app.add_flag("-b,--builtin", options.builtin, "Use builtin gmsh file reader");

  app.add_flag("-v,--verbose", options.verbose, "increase verbosity");

  // app.add_flag("-f,--force", options.force,
  //               "Force, overwrite existing ExodusII file");

  return options;
}

void msh2exo::run_msh2exo(msh2exo::Options &options) {

  if (options.version) {
    msh2exo::print_info_and_exit();
  }

  IntermediateMesh imesh;
#ifdef ENABLE_GMSH
  if (options.builtin) {
    imesh = msh2exo::read_gmsh_file(options.input_file);
  } else {
    imesh = msh2exo::read_gmsh_sdk_file(options.input_file);
  }
#else
  imesh = msh2exo::read_gmsh_file(options.input_file);
#endif
  msh2exo::write_mesh(imesh, options.output_file, options);
}

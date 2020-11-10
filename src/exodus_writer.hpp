// msh2exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2020 Weston Ortiz
//
// See the LICENSE file for license information.

#pragma once

#include <filesystem>

#include "intermediate_mesh.hpp"
#include "options.hpp"

namespace msh2exo {

void write_mesh(const IntermediateMesh &imesh,
                const std::filesystem::path &output,
                const msh2exo::Options &options);

}

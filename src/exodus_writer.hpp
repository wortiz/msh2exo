// msh2exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2020 Weston Ortiz
//
// See the LICENSE file for license information.

#pragma once

#include <filesystem>

#include "intermediate_mesh.hpp"

namespace msh2exo {

void write_mesh(const IntermediateMesh &imesh, std::filesystem::path output,
                bool force);

}

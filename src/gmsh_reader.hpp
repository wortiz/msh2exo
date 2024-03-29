// msh2exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2022 Weston Ortiz
//
// See the LICENSE file for license information.

#pragma once
#include <map>
#include <string>

#include "intermediate_mesh.hpp"

enum class gmsh_element_type {
  line2 = 1,
  tri3 = 2,
  quad4 = 3,
  tet4 = 4,
  hex8 = 5,
  line3 = 8,
  tri6 = 9,
  quad9 = 10,
  tet10 = 11,
  hex27 = 12,
  point1 = 15,
  quad8 = 16,
};

namespace msh2exo {
extern const std::map<gmsh_element_type, element_type> gmsh_type_to_elem_type;
extern const std::map<gmsh_element_type, std::vector<int>>
    gmsh_type_node_order_map;
extern const std::map<gmsh_element_type, int> gmsh_type_n_nodes;
int n_nodes_from_type(gmsh_element_type type);
IntermediateMesh read_gmsh_file(std::string filepath);
IntermediateMesh read_gmsh_sdk_file(std::string filepath);
} // namespace msh2exo

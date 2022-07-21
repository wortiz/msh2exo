// msh2exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2022 Weston Ortiz
//
// See the LICENSE file for license information.

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace msh2exo {
enum class element_type {
  line2,
  line3,
  tri3,
  tri6,
  quad4,
  quad8,
  quad9,
  hex8,
  hex27,
  tet4,
  tet10,
};

struct element_info {
  int n_nodes;
  int n_sides;
  std::vector<std::vector<int>> local_side_order;
};

struct boundary {
  int tag;
  std::string name;
  std::vector<int64_t> nodes;
};

extern const std::map<element_type, element_info> elem_info_map;

class IntermediateMesh {
public:
  int64_t dim;
  int64_t n_nodes;
  int64_t n_elements;
  int64_t n_blocks;

  struct block {
    std::string name;
    int64_t n_elements;
    element_type type;
    std::vector<int64_t> connectivity;
  };
  std::vector<double> coords;
  std::vector<block> blocks;
  std::vector<boundary> boundaries;
};
} // namespace msh2exo

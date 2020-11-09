// msh-to-exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2020 Weston Ortiz
//
// See the LICENSE file for license information.

#include "intermediate_mesh.hpp"

namespace msh2exo {
const std::map<element_type, element_info> elem_info_map = {
    {element_type::tri3, {3, 3, {{0, 1}, {1, 2}, {2, 0}}}},
    {element_type::quad4, {4, 4, {{0, 1}, {1, 2}, {2, 3}, {3, 0}}}},
    {element_type::tri6, {6, 3, {{0, 3, 1}, {1, 4, 2}, {2, 5, 0}}}},
    {element_type::quad9, {9, 4, {{0, 4, 1}, {1, 5, 2}, {2, 6, 3}, {3, 7, 0}}}},
    {element_type::tet4, {4, 4, {{0, 1, 3}, {1, 2, 3}, {0, 2, 3}, {0, 1, 2}}}},
    {element_type::hex8,
     {8,
      6,
      {{0, 1, 4, 5},
       {1, 2, 5, 6},
       {2, 3, 6, 7},
       {0, 3, 4, 7},
       {0, 1, 2, 3},
       {4, 5, 6, 7}}}},
};
}

// msh2exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2020 Weston Ortiz
//
// See the LICENSE file for license information.

#include <algorithm>
#include <ctime>
#include <fmt/printf.h>
#include <fmt/ostream.h>
#include <map>
#include <set>

extern "C" {
#include <exodusII.h>
}

#include "exodus_writer.hpp"
#include "intermediate_mesh.hpp"
#include "util.hpp"

static const std::map<msh2exo::element_type, std::string> element_type_name = {
    {msh2exo::element_type::tri3, "TRI3"},
    {msh2exo::element_type::quad4, "QUAD4"},
    {msh2exo::element_type::quad9, "QUAD9"},
    {msh2exo::element_type::tri6, "TRI6"},
    {msh2exo::element_type::hex8, "HEX8"},
    {msh2exo::element_type::tet4, "TET4"},
};

void msh2exo::write_mesh(const IntermediateMesh &imesh,
                         const std::string &output,
                         const msh2exo::Options &options) {

  int cpu_size = sizeof(double);
  int io_size = sizeof(double);
  int exoid = ex_create(output.c_str(), EX_CLOBBER, &cpu_size, &io_size);
  const char *title = "";

  std::vector<std::set<std::pair<int, int>>> elem_sides_vec(
      imesh.boundaries.size());

  int64_t offset = 0;
  int64_t elem_offset = 0;
  std::map<int64_t, std::set<int64_t>> node_elem_map;
  std::map<int64_t, size_t> elem_block_map;
  std::vector<int64_t> block_elem_start(imesh.n_blocks);
  msh2exo::print_if(options.verbose, "{}: Generating node_elem_map\n",
                    output);
  for (int i = 0; i < imesh.n_blocks; i++) {
    block_elem_start[i] = elem_offset;
    for (int j = 0; j < imesh.blocks[i].n_elements; j++) {
      int elem = elem_offset + j;
      elem_block_map.insert({elem, i});
      int n_nodes_per_elem = elem_info_map.at(imesh.blocks[i].type).n_nodes;
      for (int k = 0; k < n_nodes_per_elem; k++) {
        int node = imesh.blocks[i].connectivity[j * n_nodes_per_elem + k];
        if (node_elem_map.find(node) != node_elem_map.end()) {
          node_elem_map.at(node).insert(elem);
        } else {
          node_elem_map.insert({node, {elem}});
        }
      }
    }
    elem_offset += imesh.blocks[i].n_elements;
  }

  msh2exo::print_if(options.verbose, "{}: Generating side set pairs\n",
                    output);
  // side sets
  int n_side_sets = 0;
  for (size_t i = 0; i < imesh.boundaries.size(); i++) {
    std::set<std::pair<int, int>> &elem_sides = elem_sides_vec[i];
    std::set<int64_t> nodes(imesh.boundaries[i].nodes.begin(),
                            imesh.boundaries[i].nodes.end());
    std::set<int64_t> elem_seen;

    msh2exo::print_if(options.verbose,
                      "{}: Searching boundary {} for sides, {} nodes\n",
                      output, i, nodes.size());

    for (auto node : imesh.boundaries[i].nodes) {
      for (auto elem : node_elem_map.at(node)) {
        if (elem_seen.find(elem) == elem_seen.end()) {
          elem_seen.insert(elem);

          auto block = elem_block_map.at(elem);
          const auto &info = elem_info_map.at(imesh.blocks[block].type);

          for (size_t side = 0; side < info.n_sides; side++) {
            const auto &local_nodes = info.local_side_order[side];
            bool found = true;
            for (size_t ln = 0; ln < local_nodes.size(); ln++) {
              auto side_node =
                  imesh.blocks[block]
                      .connectivity[(elem - block_elem_start[block]) *
                                        info.n_nodes +
                                    local_nodes[ln]];
              if (nodes.find(side_node) == nodes.end()) {
                found = false;
              }
            }

            if (found) {
              elem_sides.insert({elem + 1, side + 1});
            }
          }
        }
      }
    }

    if (elem_sides.size() > 0) {
      n_side_sets++;
    }
  }

  msh2exo::print_if(options.verbose, "{}: Initializing exodus\n",
                    output);
  ex_put_init(exoid, title, imesh.dim, imesh.n_nodes, imesh.n_elements,
              imesh.n_blocks, imesh.boundaries.size(), n_side_sets);

  std::time_t tm = std::time(nullptr);

  char qa_name[] = "MSH2EXO";
  char qa_desc[] = "msh2exo";
  char date_buffer[10];
  char time_buffer[10];

  std::strftime(date_buffer, 9, "%D", std::localtime(&tm));
  std::strftime(time_buffer, 9, "%T", std::localtime(&tm));

  char *qa_record[1][4];
  qa_record[0][0] = qa_name;
  qa_record[0][1] = qa_desc;
  qa_record[0][2] = static_cast<char *>(date_buffer);
  qa_record[0][3] = static_cast<char *>(time_buffer);

  ex_put_qa(exoid, 1, qa_record);

  msh2exo::print_if(options.verbose, "{}: inserting connectivity\n",
                    output);
  offset = 0;
  for (int i = 0; i < imesh.n_blocks; i++) {
    std::vector<int> conn1(imesh.blocks[i].connectivity.size());
    for (size_t j = 0; j < conn1.size(); j++) {
      conn1[j] = imesh.blocks[i].connectivity[j] + 1;
    }
    ex_put_block(exoid, EX_ELEM_BLOCK, i + 1,
                 element_type_name.at(imesh.blocks[i].type).c_str(),
                 imesh.blocks[i].n_elements,
                 elem_info_map.at(imesh.blocks[i].type).n_nodes, 0, 0, 0);
    ex_put_name(exoid, EX_ELEM_BLOCK, i + 1, imesh.blocks[i].name.c_str());
    ex_put_conn(exoid, EX_ELEM_BLOCK, i + 1, (void *)conn1.data(), NULL, NULL);
    msh2exo::print_if(options.verbose, "\t BLOCK {} (id {}): type {}, n_elements {}, n_nodes_per_elem {}\n",
        imesh.blocks[i].name,
        i+1,
        element_type_name.at(imesh.blocks[i].type),
        imesh.blocks[i].n_elements,
        elem_info_map.at(imesh.blocks[i].type).n_nodes);

  }

  std::vector<double> x_coords(imesh.n_nodes);
  std::vector<double> y_coords(imesh.n_nodes);
  std::vector<double> z_coords(imesh.n_nodes);
  for (int64_t i = 0; i < imesh.n_nodes; i++) {
    x_coords[i] = imesh.coords[i * 3];
    y_coords[i] = imesh.coords[i * 3 + 1];
    z_coords[i] = imesh.coords[i * 3 + 2];
  }

  msh2exo::print_if(options.verbose, "{}: inserting coords\n", output);
  if (imesh.dim == 1) {
    ex_put_coord(exoid, x_coords.data(), NULL, NULL);
  } else if (imesh.dim == 2) {
    ex_put_coord(exoid, x_coords.data(), y_coords.data(), NULL);
  } else {
    ex_put_coord(exoid, x_coords.data(), y_coords.data(), z_coords.data());
  }

  msh2exo::print_if(options.verbose, "{}: inserting nodesets\n",
                    output);
  // node sets
  for (size_t i = 0; i < imesh.boundaries.size(); i++) {
    ex_put_set_param(exoid, EX_NODE_SET, imesh.boundaries[i].tag,
                     imesh.boundaries[i].nodes.size(), 0);
    std::vector<int> ns_nodes(imesh.boundaries[i].nodes.begin(),
                              imesh.boundaries[i].nodes.end());
    std::transform(ns_nodes.begin(), ns_nodes.end(), ns_nodes.begin(),
                   [](auto &val) { return val + 1; });
    ex_put_set(exoid, EX_NODE_SET, imesh.boundaries[i].tag, ns_nodes.data(), 0);
    ex_put_name(exoid, EX_NODE_SET, imesh.boundaries[i].tag,
                imesh.boundaries[i].name.c_str());
    msh2exo::print_if(options.verbose, "\t NS {} (id {}): {} nodes\n",
                      imesh.boundaries[i].name, imesh.boundaries[i].tag,
                      ns_nodes.size());
  }

  msh2exo::print_if(options.verbose, "{}: inserting sidesets\n",
                    output);
  // side sets
  for (size_t i = 0; i < imesh.boundaries.size(); i++) {
    std::set<std::pair<int, int>> &elem_sides = elem_sides_vec[i];

    if (elem_sides.size() > 0) {
      std::vector<int> elems;
      std::vector<int> sides;

      for (auto &[elem, side] : elem_sides) {
        elems.push_back(elem);
        sides.push_back(side);
      }
      ex_put_set_param(exoid, EX_SIDE_SET, imesh.boundaries[i].tag,
                       sides.size(), 0);
      ex_put_set(exoid, EX_SIDE_SET, imesh.boundaries[i].tag, elems.data(),
                 sides.data());
      ex_put_name(exoid, EX_SIDE_SET, imesh.boundaries[i].tag,
                  imesh.boundaries[i].name.c_str());
      msh2exo::print_if(options.verbose, "\t SS {} (id {}): {} sides\n",
                        imesh.boundaries[i].name, imesh.boundaries[i].tag,
                        sides.size());
    }
  }

  ex_close(exoid);
}

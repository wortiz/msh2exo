// msh2exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2020 Weston Ortiz
//
// See the LICENSE file for license information.
#include "config.hpp"
#ifdef ENABLE_GMSH
#include "gmsh_reader.hpp"
#include "intermediate_mesh.hpp"
#include "util.hpp"
#include <algorithm>
#include <cmath>
#include <fmt/format.h>
#include <gmsh.h>
#include <numeric>
#include <set>

msh2exo::IntermediateMesh
msh2exo::read_gmsh_sdk_file(std::filesystem::path filepath) {
  gmsh::initialize();

  gmsh::open(filepath);

  std::vector<std::size_t> node_tags;
  std::vector<double> coords;
  std::vector<double> parametric_coord;
  gmsh::model::mesh::getNodes(node_tags, coords, parametric_coord);

  MSH2EXO_CHECK(
      node_tags.size() > 0,
      fmt::format("GMSH SDK READER: No nodes for mesh file {}",
                  filepath.c_str()));

  gmsh::vectorpair dim_tags;
  gmsh::model::getPhysicalGroups(dim_tags);

  MSH2EXO_CHECK(
      dim_tags.size() > 0,
      fmt::format("GMSH SDK READER: No physical groups for mesh file {}",
                  filepath.c_str()));

  std::vector<std::string> phys_names;
  for (auto pair : dim_tags) {
    std::string name;
    gmsh::model::getPhysicalName(pair.first, pair.second, name);
    phys_names.emplace_back(name);
  }

  std::vector<std::vector<int>> phys_group_entities(phys_names.size());

  for (size_t i = 0; i < dim_tags.size(); i++) {
    auto [dim, tag] = dim_tags[i];
    gmsh::model::getEntitiesForPhysicalGroup(dim, tag, phys_group_entities[i]);
  }

  struct phys_elem {
    std::vector<int> element_types;
    std::vector<std::vector<std::size_t>> element_tags;
    std::vector<std::vector<std::size_t>> elem_node_tags;
  };
  std::vector<std::vector<phys_elem>> phys_elems(dim_tags.size());
  for (size_t i = 0; i < dim_tags.size(); i++) {
    auto n_ents = phys_group_entities[i].size();
    phys_elems[i].resize(n_ents);
    for (size_t j = 0; j < n_ents; j++) {
      auto ent_dim = dim_tags[i].first;
      auto ent_tag = phys_group_entities[i][j];
      gmsh::model::mesh::getElements(
          phys_elems[i][j].element_types, phys_elems[i][j].element_tags,
          phys_elems[i][j].elem_node_tags, ent_dim, ent_tag);
    }
  }

  std::vector<std::vector<std::size_t>> phys_group_node_tags(phys_names.size());
  for (size_t i = 0; i < dim_tags.size(); i++) {
    auto [dim, tag] = dim_tags[i];
    std::vector<double> coord_tmp;
    gmsh::model::mesh::getNodesForPhysicalGroup(
        dim, tag, phys_group_node_tags[i], coord_tmp);
  }

  gmsh::vectorpair entity_dim_tags;
  gmsh::model::getEntities(entity_dim_tags);

  // assume largest dim represents blocks for now
  auto max_dim = std::accumulate(
      dim_tags.begin(), dim_tags.end(), 0,
      [](auto &acc, auto &val) { return std::max(acc, val.first); });

  auto n_blocks =
      std::count_if(dim_tags.begin(), dim_tags.end(),
                    [max_dim](auto &el) { return max_dim == el.first; });

  IntermediateMesh imesh;

  imesh.dim = max_dim;

  imesh.n_blocks = n_blocks;
  imesh.blocks.resize(n_blocks);
  std::vector<std::set<size_t>> node_set(n_blocks);
  std::vector<std::set<size_t>> elem_set(n_blocks);
  size_t block_index = 0;
  for (size_t i = 0; i < dim_tags.size(); i++) {
    if (dim_tags[i].first == max_dim) {
      // make sure elements are all of the same type
      MSH2EXO_CHECK(
          phys_elems[i][0].element_types.size() == 1,
          fmt::format(
              "GMSH SDK READER: More than 1 element type in physical {}",
              phys_names[i]));
      imesh.blocks[block_index].n_elements = std::accumulate(
          phys_elems[i].begin(), phys_elems[i].end(), 0ull,
          [](auto acc, auto val) { return acc + val.element_tags[0].size(); });
      imesh.blocks[block_index].type = gmsh_type_to_elem_type.at(
          static_cast<gmsh_element_type>(phys_elems[i][0].element_types[0]));
      for (size_t j = 0; j < phys_elems[i].size(); j++) {
        for (size_t k = 0; k < phys_elems[i][j].element_tags.size(); k++) {
          for (size_t m = 0; m < phys_elems[i][j].element_tags[k].size(); m++) {
            elem_set[block_index].insert(phys_elems[i][j].element_tags[k][m]);
          }
        }
      }
      for (size_t j = 0; j < phys_elems[i].size(); j++) {
        for (size_t k = 0; k < phys_elems[i][j].elem_node_tags.size(); k++) {
          for (size_t m = 0; m < phys_elems[i][j].elem_node_tags[k].size();
               m++) {
            node_set[block_index].insert(phys_elems[i][j].elem_node_tags[k][m]);
          }
        }
      }
      block_index++;
    }
  }

  std::map<size_t, int64_t> node_index_map;
  int64_t node_index = 0;
  for (size_t block = 0; block < n_blocks; block++) {
    for (auto nid : node_set[block]) {
      if (node_index_map.find(nid) == node_index_map.end()) {
        node_index_map.insert({nid, node_index++});
      }
    }
  }
  std::map<size_t, int64_t> elem_index_map;
  int64_t elem_index = 0;
  for (size_t block = 0; block < n_blocks; block++) {
    size_t start_elem_index = elem_index;
    for (auto eid : elem_set[block]) {
      if (elem_index_map.find(eid) == elem_index_map.end()) {
        elem_index_map.insert({eid, elem_index++});
      }
    }
    imesh.blocks[block].n_elements = elem_index - start_elem_index;
  }

  imesh.n_nodes = node_index;
  imesh.n_elements = elem_index;

  // generate connectivity
  for (size_t block = 0; block < n_blocks; block++) {
    imesh.blocks[block].connectivity.resize(
        imesh.blocks[block].n_elements *
        msh2exo::elem_info_map.at(imesh.blocks[block].type).n_nodes);
  }

  block_index = 0;
  int64_t start_elem = 0;
  int64_t elem_count = 0;

  for (size_t i = 0; i < dim_tags.size(); i++) {
    if (dim_tags[i].first == max_dim) {
      for (size_t j = 0; j < phys_elems[i].size(); j++) {
        for (size_t k = 0; k < phys_elems[i][j].element_tags.size(); k++) {
          auto n_nodes_per_elem =
              gmsh_type_n_nodes.at(static_cast<gmsh_element_type>(
                  phys_elems[i][j].element_types[k]));
          for (size_t m = 0; m < phys_elems[i][j].element_tags[k].size(); m++) {
            auto elem = elem_index_map.at(phys_elems[i][j].element_tags[k][m]);
            for (int n = 0; n < n_nodes_per_elem; n++) {
              imesh.blocks[block_index]
                  .connectivity[(elem - start_elem) * n_nodes_per_elem + n] =
                  node_index_map.at(
                      phys_elems[i][j]
                          .elem_node_tags[k][m * n_nodes_per_elem + n]);
            }
            elem_count++;
          }
        }
      }
      imesh.blocks[block_index].name = phys_names[i];
      start_elem = elem_count;
      elem_count = 0;
      block_index++;
    } else {
      auto tag = dim_tags[i].second;
      auto name = phys_names[i];
      std::set<int64_t> nodes;
      for (size_t j = 0; j < phys_elems[i].size(); j++) {
        for (size_t k = 0; k < phys_elems[i][j].elem_node_tags.size(); k++) {
          for (int n = 0; n < phys_elems[i][j].elem_node_tags[k].size(); n++) {
            nodes.insert(
                node_index_map.at(phys_elems[i][j].elem_node_tags[k][n]));
          }
        }
      }
      std::vector<int64_t> nodes_vector(nodes.begin(), nodes.end());
      boundary bound{tag, name, std::move(nodes_vector)};
      imesh.boundaries.push_back(std::move(bound));
    }
  }

  std::vector<double> mapped_coords(coords.size());

  for (size_t i = 0; i < node_tags.size(); i++) {
    auto tag = node_tags[i];
    auto new_index = node_index_map.at(tag);
    for (int j = 0; j < 3; j++) {
      mapped_coords[new_index * 3 + j] = coords[i * 3 + j];
    }
  }

  imesh.coords = mapped_coords;

  return imesh;
}
#endif

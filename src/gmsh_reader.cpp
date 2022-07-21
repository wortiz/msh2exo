// msh2exo is distributed under the terms of the GNU General Public License
//
// Copyright (C) 2022 Weston Ortiz
//
// See the LICENSE file for license information.

#include <algorithm>
#include <array>
#include <cassert>
#include <fmt/format.h>
#include <fstream>
#include <limits>
#include <map>
#include <numeric>
#include <set>

#include "gmsh_reader.hpp"
#include "util.hpp"

const std::map<gmsh_element_type, msh2exo::element_type>
    msh2exo::gmsh_type_to_elem_type = {
        {gmsh_element_type::line2, element_type::line2},
        {gmsh_element_type::line3, element_type::line3},
        {gmsh_element_type::quad4, element_type::quad4},
        {gmsh_element_type::quad8, element_type::quad8},
        {gmsh_element_type::quad9, element_type::quad9},
        {gmsh_element_type::tri3, element_type::tri3},
        {gmsh_element_type::tri6, element_type::tri6},
        {gmsh_element_type::hex8, element_type::hex8},
        {gmsh_element_type::hex27, element_type::hex27},
        {gmsh_element_type::tet4, element_type::tet4},
        {gmsh_element_type::tet10, element_type::tet10},
};

const std::map<gmsh_element_type, int> msh2exo::gmsh_type_n_nodes = {
    {gmsh_element_type::point1, 1}, {gmsh_element_type::line2, 2},
    {gmsh_element_type::line3, 3},  {gmsh_element_type::quad4, 4},
    {gmsh_element_type::quad8, 8},  {gmsh_element_type::quad9, 9},
    {gmsh_element_type::tri3, 3},   {gmsh_element_type::tri6, 6},
    {gmsh_element_type::hex8, 8},   {gmsh_element_type::hex27, 27},
    {gmsh_element_type::tet4, 4},   {gmsh_element_type::tet10, 10},
};

const std::map<gmsh_element_type, std::vector<int>>
    msh2exo::gmsh_type_node_order_map{
        {gmsh_element_type::tet10, {0, 1, 2, 3, 4, 5, 6, 7, 9, 8}},
    };

static bool find_line(std::ifstream &fs, const std::string &st) {
  std::string linest;
  while (std::getline(fs, linest)) {
    if (linest == st) {
      return true;
    }
  }
  return false;
}

static void seek_string(std::ifstream &fs, const std::string &sv) {
  MSH2EXO_CHECK(find_line(fs, sv),
                fmt::format("gmsh reader: string {} not found", sv));
}

static void check_version(double version) {
  MSH2EXO_CHECK(version > 4.09, "Expected Gmsh file version >= 4.1");
}

static void check_file_type(int file_type) {
  MSH2EXO_CHECK(file_type == 0, "Expected ASCII Gmsh msh file");
}

struct gmsh_physical {
  int dim;
  int tag;
  std::string name;
};

std::vector<gmsh_physical> read_physical_names(std::ifstream &infile) {
  seek_string(infile, "$PhysicalNames");

  int n_names;
  infile >> n_names;
  std::vector<gmsh_physical> phys_names(n_names);

  for (auto i = 0; i < n_names; i++) {
    infile >> phys_names[i].dim >> phys_names[i].tag >> phys_names[i].name;
  }
  seek_string(infile, "$EndPhysicalNames");
  return phys_names;
}

struct gmsh_node {
  size_t id;
  std::array<double, 3> location;
};

static std::vector<gmsh_node> read_nodes(std::ifstream &infile) {
  seek_string(infile, "$Nodes");

  size_t n_entity_blocks;
  size_t n_nodes;
  size_t min_node_tag;
  size_t max_node_tag;

  infile >> n_entity_blocks >> n_nodes >> min_node_tag >> max_node_tag;

  std::vector<gmsh_node> nodes(n_nodes);
  size_t node_index = 0;

  for (size_t ent = 0; ent < n_entity_blocks; ent++) {
    int dim;
    int tag;
    int parametric;
    size_t n_nodes_in_block;
    infile >> dim >> tag >> parametric >> n_nodes_in_block;

    for (auto index = node_index; index < (node_index + n_nodes_in_block);
         index++) {
      infile >> nodes[index].id;
    }

    for (auto index = node_index; index < (node_index + n_nodes_in_block);
         index++) {
      infile >> nodes[index].location[0] >> nodes[index].location[1] >>
          nodes[index].location[2];
    }

    node_index += n_nodes_in_block;
  }
  seek_string(infile, "$EndNodes");
  return nodes;
}

struct gmsh_element {
  int dim;
  int tag;
  size_t type;
  size_t id;
  std::vector<size_t> nodes;
};

int msh2exo::n_nodes_from_type(gmsh_element_type type) {
  switch (type) {
  case gmsh_element_type::line2:
    return 2;
  case gmsh_element_type::tri3:
    return 3;
  case gmsh_element_type::quad4:
    return 4;
  case gmsh_element_type::tet4:
    return 4;
  case gmsh_element_type::hex8:
    return 8;
  case gmsh_element_type::point1:
    return 1;
  default:
    throw std::runtime_error("Unknown element type");
  }
}

struct gmsh_element_group {
  int dim;
  int tag;
  gmsh_element_type type;
  std::vector<size_t> elem_ids;
  std::vector<size_t> connectivity;
};

struct gmsh_entity {
  int dim;
  int tag;
  std::vector<size_t> physical_tags;
};

static void read_point_entity(std::ifstream &infile, gmsh_entity &ent) {
  double x, y, z;
  size_t n_physical_tags;
  infile >> ent.tag >> x >> y >> z >> n_physical_tags;
  ent.physical_tags.resize(n_physical_tags);
  for (size_t i = 0; i < n_physical_tags; i++) {
    infile >> ent.physical_tags[i];
  }
}

static void read_entity(std::ifstream &infile, gmsh_entity &ent) {
  double min_x, min_y, min_z;
  double max_x, max_y, max_z;
  size_t n_physical_tags;
  infile >> ent.tag >> min_x >> min_y >> min_z >> max_x >> max_y >> max_z >>
      n_physical_tags;
  ent.physical_tags.resize(n_physical_tags);
  for (size_t i = 0; i < n_physical_tags; i++) {
    infile >> ent.physical_tags[i];
  }

  size_t n_bound;
  infile >> n_bound;
  int bound_tag;
  for (size_t i = 0; i < n_bound; i++) {
    infile >> bound_tag;
  }
}

static std::vector<gmsh_entity> read_entities(std::ifstream &infile) {
  seek_string(infile, "$Entities");
  std::vector<gmsh_entity> entities;

  size_t n_points, n_curves, n_surfaces, n_volumes;
  infile >> n_points >> n_curves >> n_surfaces >> n_volumes;

  entities.resize(n_points + n_curves + n_surfaces + n_volumes);

  for (size_t i = 0; i < n_points; i++) {
    read_point_entity(infile, entities[i]);
    entities[i].dim = 0;
  }

  size_t offset = n_points;
  for (size_t i = 0; i < n_curves; i++) {
    read_entity(infile, entities[i + offset]);
    entities[i].dim = 1;
  }

  offset += n_curves;
  for (size_t i = 0; i < n_surfaces; i++) {
    read_entity(infile, entities[i + offset]);
    entities[i].dim = 2;
  }

  offset += n_surfaces;
  for (size_t i = 0; i < n_volumes; i++) {
    read_entity(infile, entities[i + offset]);
    entities[i].dim = 3;
  }

  seek_string(infile, "$EndEntities");
  return entities;
}

static std::vector<gmsh_element_group> read_elements(std::ifstream &infile) {
  seek_string(infile, "$Elements");

  size_t n_entity_blocks;
  size_t n_elements;
  size_t min_element_tag;
  size_t max_element_tag;

  infile >> n_entity_blocks >> n_elements >> min_element_tag >> max_element_tag;

  std::vector<gmsh_element_group> element_groups(n_entity_blocks);

  for (size_t ent = 0; ent < n_entity_blocks; ent++) {
    size_t element_type;
    size_t n_elements_in_block;
    infile >> element_groups[ent].dim >> element_groups[ent].tag >>
        element_type >> n_elements_in_block;
    element_groups[ent].type = static_cast<gmsh_element_type>(element_type);

    element_groups[ent].elem_ids.resize(n_elements_in_block);
    int n_nodes = msh2exo::gmsh_type_n_nodes.at(element_groups[ent].type);
    element_groups[ent].connectivity.resize(n_elements_in_block * n_nodes);
    for (size_t index = 0; index < n_elements_in_block; index++) {
      infile >> element_groups[ent].elem_ids[index];
      for (int i = 0; i < n_nodes; i++) {
        infile >> element_groups[ent].connectivity[index * n_nodes + i];
      }
    }
  }
  seek_string(infile, "$EndElements");
  return element_groups;
}

msh2exo::IntermediateMesh msh2exo::read_gmsh_file(std::string filepath) {
  std::ifstream infile(filepath);

  seek_string(infile, "$MeshFormat");
  double version;
  int file_type;
  int data_size;

  infile >> version >> file_type >> data_size;

  check_version(version);
  check_file_type(file_type);

  seek_string(infile, "$EndMeshFormat");

  auto physical_names = read_physical_names(infile);

  auto entities = read_entities(infile);

  std::map<int, std::set<int>> phys_ents;

  for (auto ent : entities) {
    for (auto phys_tag : ent.physical_tags) {
      if (phys_ents.find(phys_tag) == phys_ents.end()) {
        assert(static_cast<int>(phys_tag) < std::numeric_limits<int>::max());
        phys_ents.insert({static_cast<int>(phys_tag), {ent.tag}});
      } else {
        phys_ents[phys_tag].insert(ent.tag);
      }
    }
  }

  auto nodes = read_nodes(infile);

  auto element_groups = read_elements(infile);

  // assume largest dim represents blocks for now
  auto max_dim = std::accumulate(
      physical_names.begin(), physical_names.end(), 0,
      [](auto &acc, auto &val) { return std::max(acc, val.dim); });

  auto n_blocks =
      std::count_if(physical_names.begin(), physical_names.end(),
                    [max_dim](auto &el) { return max_dim == el.dim; });

  auto n_boundaries =
      std::count_if(physical_names.begin(), physical_names.end(),
                    [max_dim](auto &el) { return max_dim > el.dim; });

  IntermediateMesh imesh;
  imesh.dim = max_dim;
  imesh.n_blocks = n_blocks;
  imesh.blocks.resize(n_blocks);
  imesh.boundaries.resize(n_boundaries);
  imesh.n_nodes = nodes.size();
  imesh.coords.resize(imesh.n_nodes * 3);

  std::map<size_t, size_t> node_map;
  for (size_t i = 0; i < nodes.size(); i++) {
    node_map.insert({nodes[i].id, i});
    for (int j = 0; j < 3; j++) {
      imesh.coords[i * 3 + j] = nodes[i].location[j];
    }
  }

  size_t block_index = 0;
  size_t boundary_index = 0;
  imesh.n_elements = 0;
  for (auto physical : physical_names) {
    if (physical.dim == max_dim) {
      bool block_set = false;
      auto &ent_set = phys_ents.at(physical.tag);
      for (auto e_group : element_groups) {
        if (e_group.dim == physical.dim &&
            ent_set.find(e_group.tag) != ent_set.end()) {

          if (!block_set) {
            imesh.blocks[block_index].n_elements = e_group.elem_ids.size();
            imesh.blocks[block_index].type =
                msh2exo::gmsh_type_to_elem_type.at(e_group.type);
            int n_nodes =
                msh2exo::elem_info_map.at(imesh.blocks[block_index].type)
                    .n_nodes;
            imesh.blocks[block_index].name = physical.name;
            imesh.blocks[block_index].connectivity.resize(
                imesh.blocks[block_index].n_elements * n_nodes);
            for (int64_t i = 0;
                 i < (imesh.blocks[block_index].n_elements * n_nodes); i++) {
              imesh.blocks[block_index].connectivity[i] =
                  node_map.at(e_group.connectivity[i]);
            }
            block_set = true;
          } else {
            MSH2EXO_CHECK(
                msh2exo::gmsh_type_to_elem_type.at(e_group.type) ==
                    imesh.blocks[block_index].type,
                fmt::format("More than one element type found in physical "
                            "group {} tag {}",
                            physical.name, physical.tag));
            size_t offset = imesh.blocks[block_index].n_elements;
            imesh.blocks[block_index].n_elements += e_group.elem_ids.size();
            int n_nodes =
                msh2exo::elem_info_map.at(imesh.blocks[block_index].type)
                    .n_nodes;
            imesh.blocks[block_index].connectivity.resize(
                imesh.blocks[block_index].n_elements * n_nodes);
            for (int64_t i = 0;
                 i < (imesh.blocks[block_index].n_elements * n_nodes); i++) {
              imesh.blocks[block_index].connectivity[offset * n_nodes + i] =
                  node_map.at(e_group.connectivity[i]);
            }
          }
        }
      }
      imesh.n_elements += imesh.blocks[block_index].n_elements;
      block_index++;
    } else {
      auto &ent_set = phys_ents.at(physical.tag);
      std::set<size_t> ss_nodes;
      for (auto e_group : element_groups) {
        if (e_group.dim == physical.dim &&
            ent_set.find(e_group.tag) != ent_set.end()) {
          for (size_t i = 0; i < e_group.connectivity.size(); i++) {
            ss_nodes.insert(node_map.at(e_group.connectivity[i]));
          }
        }
      }
      imesh.boundaries[boundary_index].name = physical.name;
      imesh.boundaries[boundary_index].tag = physical.tag;
      imesh.boundaries[boundary_index].nodes.resize(ss_nodes.size());
      std::copy(ss_nodes.begin(), ss_nodes.end(),
                imesh.boundaries[boundary_index].nodes.begin());
      boundary_index++;
    }
  }

  return imesh;
}

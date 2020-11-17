# msh2exo

A Gmsh (Gmsh msh format) to Exodus (ExodusII format) mesh conversion utility

# License

msh2exo is distributed under the terms of the GNU General Public License

Copyright (C) 2020 Weston Ortiz

See the LICENSE file for license information. 

## Third Party Libraries

Various third party libraries are included in `tpls/` please see their
respective licenses for usage

# Requirements

- CMake >= 3.13 (binaries available at
  [https://cmake.org/download/](https://cmake.org/download/))
- ExodusII library
  [https://github.com/gsjaardema/seacas](https://github.com/gsjaardema/seacas)
- (optional) Gmsh SDK (available prebuilt through pip or the Gmsh website
  [https://gmsh.info/#Download](https://gmsh.info/#Download))
  when built with Gmsh SDK, Gmsh will be used to read the msh files
- C++14 compatible compiler (e.g. GCC > 5)

# Building

```sh
$ git clone --recursive https://github.com/wortiz/msh2exo
$ mkdir msh2exo/build
$ cd msh2exo/build
$ cmake .. -DSEACASExodus_DIR=<path to exodus>/lib/cmake/SEACASExodus \
     -DGmsh_DIR=<path to gmsh sdk> # optional
$ make # should create a msh2exo executable
```

# Usage

```sh
msh2exo: mesh conversion utility
Usage: ./build/msh2exo [OPTIONS] input_file output_file

Positionals:
  input_file TEXT:FILE REQUIRED
                              Input (Gmsh msh) mesh file
  output_file TEXT REQUIRED   Output (ExodusII) mesh file

Options:
  -h,--help                   Print this help message and exit
  -V,--version                print version and basic info
  -b,--builtin                Use builtin gmsh file reader
  -v,--verbose                increase verbosity

```

# Examples

Examples are available at [msh2exo-examples](https://github.com/wortiz/msh2exo-examples)

# Supported Elements

- Quadrilateral elements (First and Second Order: `QUAD4`, `QUAD9`)
- Triangular elements (First and Second Order: `TRI3`, `TRI6`)
- Tetrahedral elements (First Order: `TET4`)
- Hexahedral elements (First Order: `HEX8`)

# Conversion from Physical Groups to ExodusII data

- ExodusII Blocks are created from physical groups with the largest dimension
  (MAX\_DIM)
  - e.g. if you have a physical volume, a physical surface, and a physical
    curve then the volume will represent an exodus block

- ExodusII Sidesets are created from physical groups of dimension MAX\_DIM - 1,
  a nodeset of the same name and id will also be created

- ExodusII Nodesets will be created from physical groups of dimensions less
  than MAX\_DIM

# ExodusII data numbering

- ExodusII nodes are renumbered to 1 to N\_Nodes

- ExodusII elements are renumbered to 1 to N\_Elements

- ExodusII blocks are renumbered to 1 to N\_Blocks

- ExodusII sidesets and nodesets keep the same number as the physical group

# ExodusII data naming

- Names should be preserved for blocks, sidesets, and nodesets

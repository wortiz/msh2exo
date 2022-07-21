# msh2exo

A Gmsh (Gmsh msh format) to Exodus (ExodusII format) mesh conversion utility

# License

msh2exo is distributed under the terms of the GNU General Public License

Copyright (C) 2022 Weston Ortiz

See the LICENSE file for license information. 

## Third Party Libraries

Various third party libraries are included in `tpls/` please see their
respective licenses for usage

# Requirements

- CMake >= 3.13 (binaries available at
  [https://cmake.org/download/](https://cmake.org/download/))
- ExodusII library
  [https://github.com/sandialabs/seacas](https://github.com/sandialabs/seacas)
- (optional) Gmsh SDK (available prebuilt through pip or the Gmsh website
  [https://gmsh.info/#Download](https://gmsh.info/#Download))
  when built with Gmsh SDK, Gmsh will be used to read the msh files
- C++14 compatible compiler (e.g. GCC > 5)

# Building

1.  A working SEACASExodus install is required

    See build instructions in the SEACAS repository:

    [https://github.com/sandialabs/seacas/#get-the-sources](https://github.com/sandialabs/seacas/#get-the-sources)

    Or Build with spack [https://github.com/sandialabs/seacas/#spack](https://github.com/sandialabs/seacas/#spack)
    
    spack install seacas
    spack load seacas

2. Instructions for building `msh2exo`

The following commands show an example build:

`<path to seacas>` is the pass where SEACAS was installed, usually the seacas git folder, 
if installed with spack this can be skipped

`<install prefix>` is where you want the executable, will install to `<install prefix>/bin`

`<path to gmsh sdk>` is the path to a downloaded gmsh SDK which contains include/gmsh.h and 
the gmsh library under the lib folder.

```sh
$ git clone --recursive https://github.com/wortiz/msh2exo
$ mkdir msh2exo/build
$ cd msh2exo/build
$ cmake .. -DSEACASExodus_DIR=<path to seacas>/lib/cmake/SEACASExodus \
     -DCMAKE_INSTALL_PREFIX=<install prefix> \ # optional
     -DGmsh_DIR=<path to gmsh sdk> # optional
$ make # should create a msh2exo executable in msh2exo/build folder
$ make install # optional, will install to <install prefix>/bin
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

- Quadrilateral elements (First and Second Order: `QUAD4`, `QUAD8`, `QUAD9`)
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

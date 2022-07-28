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

# Building

For notes on building for windows see [Windows Build Notes](#windows-build-notes)

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

## Windows Build Notes

A working build for Windows 10 with Visual Studio 2019 was done as follows:

Prerequisites:

[Git](https://git-scm.com/)
[vcpkg](https://vcpkg.io/en/index.html)
[Visual Studio](https://visualstudio.microsoft.com/) tested with 2019 community
[CMake](https://cmake.org/)

My vcpkg was installed to `C:\src\vcpkg` the following will need to be adjusted if another location is used
I did `vcpkg integrate install` to integrate with Visual Studio

1. Install Exodus prerequisites:

   This was done in powershell terminal from `C:\src\vcpkg`

   ```
   .\vcpkg install fmt --triplet=x64-windows
   .\vcpkg install netcdf --triplet=x64-windows
   ```

2. Build and Install Exodus

   In C:\src I cloned SEACAS

   ```
   cd C:\src
   git clone https://github.com/sandialabs/seacas
   ```

   Then using CMake configure SEACAS to build ExodusII, modified from appveyor config in SEACAS repository.
   Adjust locations specified with C:\ to preferred locations

   If cmake is not on path use full path to cmake.exe

   ```
   cd C:\src\seacas
   
   mkdir build

   cd build

   cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/Tools/vcpkg/scripts/buildsystems/vcpkg.cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/wd4478 /EHsc" -DCMAKE_C_FLAGS="/wd4478" -DCMAKE_INSTALL_PREFIX=C:\src\seacas\install -DHDF5_ROOT:PATH=C:\src\vcpkg\installed\x64-windows -DMatio_INCLUDE_DIRS:PATH=C:\src\vcpkg\installed\x64-windows\include -DNetCDF_ROOT:PATH=C:\src\vcpkg\installed\x64-windows -DSEACASExodus_ENABLE_THREADSAFE:BOOL=OFF -DSEACASIoss_ENABLE_THREADSAFE:BOOL=OFF -DSeacas_ENABLE_SEACASExodus:BOOL=ON -DSeacas_ENABLE_DOXYGEN:BOOL=OFF -DSeacas_ENABLE_Fortran=OFF -DSeacas_ENABLE_TESTS=OFF -DSeacas_ENABLE_Zoltan:BOOL=OFF -DSeacas_HIDE_DEPRECATED_CODE:BOOL=ON -DSeacas_SKIP_FORTRANCINTERFACE_VERIFY_TEST:BOOL=ON -DTPL_ENABLE_ADIOS2:BOOL=OFF -DTPL_ENABLE_CGNS:BOOL=OFF -DTPL_ENABLE_Kokkos:BOOL=OFF -DTPL_ENABLE_MPI:BOOL=OFF -DTPL_ENABLE_Matio:BOOL=OFF -DTPL_ENABLE_Netcdf:BOOL=ON -DTPL_ENABLE_Pamgen:BOOL=OFF -DTPL_ENABLE_Pthread:BOOL=OFF -DTPL_ENABLE_X11:BOOL=OFF -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DTPL_ENABLE_gtest:BOOL=OFF
   ```

   Building Exodus using CMake (from within build directory where cmake was run above):

   ```
   cmake --build . --config Release
   ```

   Installing Exodus to CMAKE_INSTALL_PREFIX using CMake (again from build directory):

   ```
   cmake --install . --config Release
   ```

3. Building `msh2exo`

   Clone repo

   ```
   cd C:\src

   git clone --recursive https://github.com/wortiz/msh2exo
   ```

   configure `msh2exo` (adjust paths if necessary)

   ```
   cd C:\src\msh2exo
   
   mkdir build
   
   cd build

   cmake .. -DSEACASExodus_DIR=C:\src\seacas\install\lib\cmake\SEACASExodus -DCMAKE_INSTALL_PREFIX=C:\src\msh2exo\install -G "Visual Studio 16 2019" -A x64
   ```

   Build `msh2exo` (from build directory)

   ```
   cmake --build . --config Release
   ```

   Installing `msh2exo` to CMAKE_INSTALL_PREFIX using CMake (again from build directory):

   ```
   cmake --install . --config Release
   ```
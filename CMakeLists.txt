cmake_minimum_required(VERSION 3.13)
project(msh2exo VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

list(APPEND CMAKE_MODULE_PATH "${msh2exo_SOURCE_DIR}/cmake")

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release")
endif()

find_package(SEACASExodus REQUIRED HINTS ${ACCESS}/lib/cmake/SEACASExodus)
find_package(Gmsh 4.6)

add_subdirectory(tpls/fmt)
add_subdirectory(tpls/CLI11)

set(MSH2EXO_THIRD_PARTY_LIBS
  fmt::fmt
  CLI11::CLI11
  ${SEACASExodus_LIBRARIES}
  ${SEACASExodus_TPL_LIBRARIES})

set(MSH2EXO_THIRD_PARTY_INCLUDES
  ${SEACASExodus_INCLUDE_DIRS}
  ${SEACASExodus_TPL_INCLUDE_DIRS})

if (Gmsh_FOUND)
  set(ENABLE_GMSH ON CACHE BOOL "Enable Gmsh SDK integration")
  if (ENABLE_GMSH)
    set(MSH2EXO_THIRD_PARTY_LIBS
      ${MSH2EXO_THIRD_PARTY_LIBS}
      ${Gmsh_LIBRARIES})

    set(MSH2EXO_THIRD_PARTY_INCLUDES
      ${MSH2EXO_THIRD_PARTY_INCLUDES}
      ${Gmsh_INCLUDE_DIRS})
  endif()
endif()

set(ENABLE_SOURCE_LOCATION OFF CACHE BOOL "Enable experminental source location library")

set(msh2exo_SOURCES
    gmsh_reader.hpp
    gmsh_reader.cpp
    gmsh_sdk_reader.cpp
    intermediate_mesh.cpp
    intermediate_mesh.hpp
    exodus_writer.hpp
    exodus_writer.cpp
    options.hpp
    options.cpp
    util.hpp
    util.cpp)
list(TRANSFORM msh2exo_SOURCES PREPEND src/)

configure_file(src/config.hpp.in
  ${PROJECT_BINARY_DIR}/include/config.hpp)

add_executable(msh2exo src/main.cpp ${msh2exo_SOURCES})

target_include_directories(msh2exo PUBLIC src
  ${PROJECT_BINARY_DIR}/include 
  ${MSH2EXO_THIRD_PARTY_INCLUDES})

target_link_directories(msh2exo PUBLIC
  ${SEACASExodus_LIBRARY_DIRS}
  ${SEACASExodus_TPL_LIBRARY_DIRS})

target_link_libraries(msh2exo PUBLIC ${MSH2EXO_THIRD_PARTY_LIBS})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(msh2exo PRIVATE
    -Wall -Wextra -pedantic -Wshadow)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(msh2exo PRIVATE
    -Wall -Wextra -pedantic -Wshadow)
endif()

install(TARGETS msh2exo RUNTIME DESTINATION bin)

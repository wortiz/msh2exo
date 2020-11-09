find_path(Gmsh_INCLUDE_DIRS NAMES gmsh.h gmshc.h
  HINTS
  /usr/include/
  /usr/local/include/
  ${GMSH_DIR}/include/
  ${Gmsh_DIR}/include/
  ${GMSH_INCLUDE_DIR}
  ${Gmsh_INCLUDE_DIR}
  PATH_SUFFIXES
  gmsh
)

find_library(Gmsh_LIBRARIES NAMES gmsh libgmsh
  HINTS
  /usr/lib
  /usr/local/lib
  /usr/lib64
  /usr/local/lib64
  ${GMSH_DIR}/lib
  ${Gmsh_DIR}/lib
  ${GMSH_LIBRARY_DIR}
  ${Gmsh_LIBRARY_DIR}
  PATH_SUFFIXES
  gmsh
)

if(Gmsh_INCLUDE_DIRS)
  if(EXISTS ${Gmsh_INCLUDE_DIRS}/gmsh.h)
    file(READ ${Gmsh_INCLUDE_DIRS}/gmsh.h GMSH_HEADER_FILE)
    string(REGEX MATCH "\#define GMSH_API_VERSION *\"([0-9,.]*).*\"" GMSH_VERSION_STRING ${GMSH_HEADER_FILE})
    set(Gmsh_VERSION ${CMAKE_MATCH_1} CACHE INTERNAL "Gmsh Version")
  else()
    message(SEND_ERROR "Could not find " gmsh.h " in " ${Gmsh_INCLUDE_DIR} ".")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gmsh
  FOUND_VAR Gmsh_FOUND
  REQUIRED_VARS Gmsh_LIBRARIES Gmsh_INCLUDE_DIRS
  VERSION_VAR Gmsh_VERSION)

mark_as_advanced(
  Gmsh_INCLUDE_DIRS
  Gmsh_LIBRARIES
)


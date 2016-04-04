#.rst:
# FindGMP
# -------
#
# Find the GNU Multiple Precision Arithmetic Library (GMP).
#
# This module searches for the C header gmp.h and the GMP C library. If
# specified, this module also searches for the following component:
#
# ``C++`` or ``CXX``
#   The C++ interface to GMP, which is defined in the C++ header gmpxx.h and
#   requires linking to a separate GMP C++ library.
#
# The following variables will be defined:
#
# ``GMP_FOUND``
#   Set to true if the C header and library and all required components are
#   found, false otherwise.
# ``GMP_INCLUDE_DIRS``
#   The include directories for GMP's C interface.
# ``GMP_LIBRARIES``
#   The libraries to link against to use GMP's C interface.
# ``GMP_<component>_FOUND``
#   Set to true if <component> is found.
# ``GMP_<component>_INCLUDE_DIRS``
#   The include directories for <component>.
# ``GMP_<component>_LIBRARIES``
#   The libraries to link against to use <component>.
# ``GMP_VERSION_STRING``
#   A human-readable string containing GMP's version number.
# ``GMP_VERSION_MAJOR``
#   GMP's major version.
# ``GMP_VERSION_MINOR``
#   GMP's minor version.
# ``GMP_VERSION_PATCH``
#   GMP's patch version.

foreach(spelling C++ CXX)
  list(FIND GMP_FIND_COMPONENTS ${spelling} _GMP_FIND_COMPONENTS_INDEX_CXX)
  if(NOT _GMP_FIND_COMPONENTS_INDEX_CXX EQUAL -1)
    list(GET GMP_FIND_COMPONENTS ${_GMP_FIND_COMPONENTS_INDEX_CXX}
      _GMP_COMPONENT_CXX)
    break()
  endif()
endforeach()

find_path(GMP_INCLUDE_DIR
          gmp.h
          DOC "Path to the directory containing the C header <gmp.h>.")
find_library(GMP_LIBRARY
             gmp
             DOC "Path to the GMP C library.")
mark_as_advanced(GMP_INCLUDE_DIR GMP_LIBRARY)

if(NOT _GMP_FIND_COMPONENTS_INDEX_CXX EQUAL -1)
  find_path(GMP_${_GMP_COMPONENT_CXX}_INCLUDE_DIR
            gmpxx.h
            DOC "Path to the directory containing the C++ header <gmpxx.h>.")
  find_library(GMP_${_GMP_COMPONENT_CXX}_LIBRARY
               gmpxx
               DOC "Path to the GMP C++ library.")
  mark_as_advanced(GMP_${_GMP_COMPONENT_CXX}_INCLUDE_DIR
    GMP_${_GMP_COMPONENT_CXX}_LIBRARY)
endif()

set(GMP_INCLUDE_DIRS)
set(GMP_LIBRARIES)
if(GMP_INCLUDE_DIR)
  set(GMP_INCLUDE_DIRS ${GMP_INCLUDE_DIR})
endif()
if(GMP_LIBRARY)
  set(GMP_LIBRARIES ${GMP_LIBRARY})
endif()
if(NOT _GMP_FIND_COMPONENTS_INDEX_CXX EQUAL -1)
  set(GMP_${_GMP_COMPONENT_CXX}_INCLUDE_DIRS)
  set(GMP_${_GMP_COMPONENT_CXX}_LIBRARIES)
  if(GMP_${_GMP_COMPONENT_CXX}_INCLUDE_DIR)
    set(GMP_${_GMP_COMPONENT_CXX}_INCLUDE_DIRS
      ${GMP_INCLUDE_DIRS} ${GMP_${_GMP_COMPONENT_CXX}_INCLUDE_DIR})
    list(REMOVE_DUPLICATES GMP_${_GMP_COMPONENT_CXX}_INCLUDE_DIRS)
  endif()
  if(GMP_${_GMP_COMPONENT_CXX}_LIBRARY)
    set(GMP_${_GMP_COMPONENT_CXX}_LIBRARIES
      ${GMP_LIBRARIES} ${GMP_${_GMP_COMPONENT_CXX}_LIBRARY})
  endif()
endif()

if(GMP_INCLUDE_DIR AND EXISTS ${GMP_INCLUDE_DIR}/gmp.h)
  file(STRINGS ${GMP_INCLUDE_DIR}/gmp.h _GMP_VERSION_STRINGS
       REGEX "^#define[ \t]+__GNU_MP_VERSION(_MINOR|_PATCHLEVEL)?")
  foreach(gmp_version_string ${_GMP_VERSION_STRINGS})
    if(gmp_version_string MATCHES "^#define[ \t]+__GNU_MP_VERSION[ \t]+(.*)")
      set(GMP_VERSION_MAJOR ${CMAKE_MATCH_1})
    elseif(gmp_version_string MATCHES "^#define[ \t]+__GNU_MP_VERSION_MINOR[ \t]+(.*)")
      set(GMP_VERSION_MINOR ${CMAKE_MATCH_1})
    elseif(gmp_version_string MATCHES "^#define[ \t]+__GNU_MP_VERSION_PATCHLEVEL[ \t]+(.*)")
      set(GMP_VERSION_PATCH ${CMAKE_MATCH_1})
    endif()
  endforeach()
  set(GMP_VERSION_STRING ${GMP_VERSION_MAJOR}.${GMP_VERSION_MINOR}.${GMP_VERSION_PATCH})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP
                                  REQUIRED_VARS GMP_LIBRARY GMP_INCLUDE_DIR
                                  VERSION_VAR GMP_VERSION_STRING
                                  HANDLE_COMPONENTS)

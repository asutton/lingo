#.rst:
# FindGMP
# -------
#
# Find the GNU Multiple Precision Arithmetic Library (GMP).
#
# The following variables will be defined:
#
# ``GMP_FOUND``
#   Set to true if the C header and library were found.
# ``GMP_INCLUDE_DIRS``
#   The include directories for GMP's C interface.
# ``GMP_LIBRARIES``
#   The libraries to link against to use GMP's C interface.
# ``GMPXX_FOUND``
#   Set to true if the C++ header and library were found.
# ``GMPXX_INCLUDE_DIRS``
#   The include directories for GMP's C++ interface.
# ``GMPXX_LIBRARIES``
#   The libraries to link against to use GMP's C++ interface.
# ``GMP_VERSION_STRING``
#   A human-readable string containing GMP's version number.
# ``GMP_VERSION_MAJOR``
#   GMP's major version.
# ``GMP_VERSION_MINOR``
#   GMP's minor version.
# ``GMP_VERSION_PATCH``
#   GMP's patch version.

find_path(GMP_INCLUDE_DIR
          gmp.h
          DOC "Path to the directory containing the C header <gmp.h>.")
find_path(GMPXX_INCLUDE_DIR
          gmpxx.h
          DOC "Path to the directory containing the C++ header <gmpxx.h>.")

find_library(GMP_LIBRARY
             gmp
             DOC "Path to the GMP C library.")
find_library(GMPXX_LIBRARY
             gmpxx
             DOC "Path to the GMP C++ library.")

if(GMP_INCLUDE_DIR)
  set(GMP_INCLUDE_DIRS ${GMP_INCLUDE_DIR})
  set(GMPXX_INCLUDE_DIRS ${GMP_INCLUDE_DIR})
endif()
if(GMP_LIBRARY)
  set(GMP_LIBRARIES ${GMP_LIBRARY})
  set(GMPXX_LIBRARIES ${GMP_LIBRARY})
endif()
if(GMPXX_INCLUDE_DIR)
  set(GMPXX_INCLUDE_DIRS ${GMPXX_INCLUDE_DIRS} ${GMPXX_INCLUDE_DIR})
  list(REMOVE_DUPLICATES GMPXX_INCLUDE_DIRS)
endif()
if(GMPXX_LIBRARY)
  set(GMPXX_LIBRARIES ${GMPXX_LIBRARIES} ${GMPXX_LIBRARY})
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
                                  VERSION_VAR GMP_VERSION_STRING)

if(GMPXX_INCLUDE_DIR AND GMPXX_LIBRARY)
  set(GMPXX_FOUND TRUE)
else()
  set(GMPXX_FOUND FALSE)
endif()

mark_as_advanced(GMP_INCLUDE_DIR GMP_LIBRARY GMPXX_INCLUDE_DIR GMPXX_LIBRARY)

#.rst:
# FindICONV
# ---------
#
# Find the iconv library.
#
# This module tests for the presence of the POSIX/XSI iconv function family in
# either the C library or a separate libiconv library.
#
# The following variables will be defined:
#
# ``ICONV_FOUND``
#   Set to true if iconv.h and a working ``iconv()`` implementation was found.
# ``ICONV_INCLUDE_DIRS``
#   The include directories.
# ``ICONV_LIBRARIES``
#   The libraries to link against.
# ``ICONV_WORKS``
#   Set to true or 1 if a working ``iconv()`` implementation was found, false
#   or empty otherwise.
# ``ICONV_CONST``
#   Set to "const" or empty, depending on whether the second argument of
#   ``iconv()`` is of type ``const char **`` or ``char **``. Left undefined if
#   ``iconv()`` was not found.

include(CMakePushCheckState)
include(CheckPrototypeDefinition)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)

find_path(
  ICONV_INCLUDE_DIR
  iconv.h
  DOC "Path to the directory containing the header <iconv.h>."
)

find_library(
  ICONV_LIBRARY
  NAMES iconv libiconv libiconv2 libiconv-2
  DOC "Path to the iconv library."
)

if(ICONV_INCLUDE_DIR)
  cmake_push_check_state()

  set(_HAVE_ICONV_SOURCE "
#include <stdlib.h>
#include <iconv.h>

int main() {
  iconv_t cd = iconv_open(\"\", \"\");
  iconv(cd, NULL, NULL, NULL, NULL);
  iconv_close(cd);
  return 0;
}
")

  # Check whether iconv() is in the C library.
  set(CMAKE_REQUIRED_INCLUDES ${ICONV_INCLUDE_DIR})
  check_c_source_compiles("${_HAVE_ICONV_SOURCE}" HAVE_LIBC_ICONV)

  if(NOT HAVE_LIBC_ICONV AND ICONV_LIBRARY)
    # Check whether iconv() is in libiconv.
    set(CMAKE_REQUIRED_LIBRARIES ${ICONV_LIBRARY})
    check_c_source_compiles("${_HAVE_ICONV_SOURCE}" HAVE_LIBICONV_ICONV)
  endif()

  if(HAVE_LIBC_ICONV OR HAVE_LIBICONV_ICONV)
    # Check whether iconv() works.
    # This tests against bugs in AIX 5.1, AIX 6.1 to 7.1, HP-UX 11.11, and
    # Solaris 10.
    # (Test source taken from iconv.m4 in the GNU gettext package.)
    if(NOT CMAKE_CROSSCOMPILING)
      check_c_source_runs("
#include <string.h>
#include <iconv.h>

int main() {
  int result = 0;
  /* Test against AIX 5.1 bug: Failures are not distinguishable from successful
     returns.  */
  {
    iconv_t cd_utf8_to_88591 = iconv_open (\"ISO8859-1\", \"UTF-8\");
    if (cd_utf8_to_88591 != (iconv_t)(-1)) {
      static const char input[] = \"\\\\342\\\\202\\\\254\"; /* EURO SIGN */
      char buf[10];
      const char *inptr = input;
      size_t inbytesleft = strlen (input);
      char *outptr = buf;
      size_t outbytesleft = sizeof (buf);
      size_t res = iconv (cd_utf8_to_88591,
                          (char **) &inptr, &inbytesleft,
                          &outptr, &outbytesleft);
      if (res == 0)
        result |= 1;
      iconv_close (cd_utf8_to_88591);
    }
  }
  /* Test against Solaris 10 bug: Failures are not distinguishable from
     successful returns.  */
  {
    iconv_t cd_ascii_to_88591 = iconv_open (\"ISO8859-1\", \"646\");
    if (cd_ascii_to_88591 != (iconv_t)(-1)) {
      static const char input[] = \"\\\\263\";
      char buf[10];
      const char *inptr = input;
      size_t inbytesleft = strlen (input);
      char *outptr = buf;
      size_t outbytesleft = sizeof (buf);
      size_t res = iconv (cd_ascii_to_88591,
                          (char **) &inptr, &inbytesleft,
                          &outptr, &outbytesleft);
      if (res == 0)
        result |= 2;
      iconv_close (cd_ascii_to_88591);
    }
  }
  /* Test against AIX 6.1..7.1 bug: Buffer overrun.  */
  {
    iconv_t cd_88591_to_utf8 = iconv_open (\"UTF-8\", \"ISO-8859-1\");
    if (cd_88591_to_utf8 != (iconv_t)(-1)) {
      static const char input[] = \"\\\\304\";
      static char buf[2] = { (char)0xDE, (char)0xAD };
      const char *inptr = input;
      size_t inbytesleft = 1;
      char *outptr = buf;
      size_t outbytesleft = 1;
      size_t res = iconv (cd_88591_to_utf8,
                          (char **) &inptr, &inbytesleft,
                          &outptr, &outbytesleft);
      if (res != (size_t)(-1) || outptr - buf > 1 || buf[1] != (char)0xAD)
        result |= 4;
      iconv_close (cd_88591_to_utf8);
    }
  }
#if 0 /* This bug could be worked around by the caller.  */
  /* Test against HP-UX 11.11 bug: Positive return value instead of 0.  */
  {
    iconv_t cd_88591_to_utf8 = iconv_open (\"utf8\", \"iso88591\");
    if (cd_88591_to_utf8 != (iconv_t)(-1)) {
      static const char input[] = \"\\\\304rger mit b\\\\366sen B\\\\374bchen ohne Augenma\\\\337\";
      char buf[50];
      const char *inptr = input;
      size_t inbytesleft = strlen (input);
      char *outptr = buf;
      size_t outbytesleft = sizeof (buf);
      size_t res = iconv (cd_88591_to_utf8,
                          (char **) &inptr, &inbytesleft,
                          &outptr, &outbytesleft);
      if ((int)res > 0)
        result |= 8;
      iconv_close (cd_88591_to_utf8);
    }
  }
#endif
  /* Test against HP-UX 11.11 bug: No converter from EUC-JP to UTF-8 is
     provided.  */
  if (/* Try standardized names.  */
      iconv_open (\"UTF-8\", \"EUC-JP\") == (iconv_t)(-1)
      /* Try IRIX, OSF/1 names.  */
      && iconv_open (\"UTF-8\", \"eucJP\") == (iconv_t)(-1)
      /* Try AIX names.  */
      && iconv_open (\"UTF-8\", \"IBM-eucJP\") == (iconv_t)(-1)
      /* Try HP-UX names.  */
      && iconv_open (\"utf8\", \"eucJP\") == (iconv_t)(-1))
    result |= 16;
  return result;
}
" ICONV_WORKS)
    else()
      # Guess value for ICONV_WORKS based on the name of the host system.
      if(CMAKE_HOST_SYSTEM_NAME STREQUAL "AIX" OR CMAKE_HOST_SYSTEM_NAME STREQUAL "HP-UX")
        set(ICONV_WORKS FALSE CACHE BOOL "Whether iconv() works on the host system (set to false on AIX and HP-UX by default).")
      else()
        set(ICONV_WORKS TRUE CACHE BOOL "Whether iconv() works on the host system (set to true by default).")
      endif()
      if(ICONV_WORKS)
        message(STATUS "Performing Test ICONV_WORKS - success (guessed)")
      else()
        message(STATUS "Performing Test ICONV_WORKS - failed (guessed)")
      endif()
    endif()
    set(HAVE_ICONV ${ICONV_WORKS})

    # Check whether the second argument of iconv() is declared as 'const'.
    check_prototype_definition(
      iconv
      "size_t iconv(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft)"
      0
      "stdlib.h;iconv.h"
      ICONV_PROTOTYPE_ARG1
    )

    if(ICONV_PROTOTYPE_ARG1)
      set(ICONV_CONST "")
    else()
      set(ICONV_CONST "const")
    endif()
  endif()

  cmake_pop_check_state()
endif()

set(_ICONV_REQUIRED_VARS ICONV_INCLUDE_DIR ICONV_WORKS)

if(ICONV_INCLUDE_DIR)
  set(ICONV_INCLUDE_DIRS ${ICONV_INCLUDE_DIR})
endif()

if(ICONV_LIBRARY)
  set(ICONV_LIBRARIES ${ICONV_LIBRARY})
  set(_ICONV_REQUIRED_VARS ICONV_LIBRARY ${_ICONV_REQUIRED_VARS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ICONV DEFAULT_MSG ${_ICONV_REQUIRED_VARS})

mark_as_advanced(ICONV_INCLUDE_DIR ICONV_LIBRARY)

if(CMAKE_CROSSCOMPILING AND ICONV_INCLUDE_DIR)
  mark_as_advanced(ICONV_WORKS)
endif()

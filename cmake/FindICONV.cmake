#.rst:
# FindICONV
# ---------
#
# Find the iconv library.
#
# This module tests for the presence of the POSIX/XSI iconv function family
# in either the C library or a separate libiconv library.
#
# The following variables will be defined:
#
# ``ICONV_FOUND``
#   Set to true if the C header iconv.h and a working ``iconv()``
#   implementation are found, false otherwise.
# ``ICONV_INCLUDE_DIRS``
#   The include directories where the C header iconv.h is found.
# ``ICONV_LIBRARIES``
#   The libraries to link against to use ``iconv()``.
# ``HAVE_ICONV``
#   Set to true if ``ICONV_FOUND`` is true, false otherwise.
#
# If the ``iconv()`` function exists, the following variables are also
# defined:
#
# ``ICONV_CONST``
#   Set to "const" or empty, depending on whether the second argument of
#   ``iconv()`` is of type ``const char **`` or ``char **``.
# ``ICONV_WORKS``
#   Set to true or 1 if a working ``iconv()`` implementation was found, false
#   or empty otherwise.
# ``HAVE_GNU_ICONV``
#   Set to true or 1 if either GNU libiconv or GNU libc is installed, false
#   or empty otherwise.
# ``ICONV_SUPPORTS_UNICODE``
#   Set to true or 1 if ``iconv()`` supports conversion between UTF-8 and the
#   big endian and little endian variants of UTF-16 and UTF-32, false or
#   empty otherwise.
#
# Based on the `Gnulib <https://www.gnu.org/software/gnulib/>`_ modules
# 'iconv', 'iconv_open', and 'iconv_open-utf'.

include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckPrototypeDefinition)
include(CMakePushCheckState)

find_path(ICONV_INCLUDE_DIR
          iconv.h
          DOC "Path to the directory containing the header <iconv.h>.")
find_library(ICONV_LIBRARY
             NAMES iconv iconv2 iconv-2
             DOC "Path to the iconv library.")
mark_as_advanced(ICONV_INCLUDE_DIR ICONV_LIBRARY)

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
    set(CMAKE_REQUIRED_LIBRARIES)
  endif()

  if(HAVE_LIBC_ICONV OR HAVE_LIBICONV_ICONV)
    if(HAVE_LIBICONV_ICONV)
      set(CMAKE_REQUIRED_LIBRARIES ${ICONV_LIBRARY})
    endif()

    # Check whether the second argument of iconv() is declared as 'const'.
    check_prototype_definition(iconv
      "size_t iconv(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft)"
      0
      "stdlib.h;iconv.h"
      ICONV_PROTOTYPE_ARG1)

    if(ICONV_PROTOTYPE_ARG1)
      set(ICONV_CONST "")
    else()
      set(ICONV_CONST "const")
    endif()

    # Check whether iconv() works, i.e. this tests against bugs in AIX 5.1,
    # AIX 6.1 to 7.1, HP-UX 11.11, and Solaris 10.
    if(NOT CMAKE_CROSSCOMPILING)
      check_c_source_runs("
#include <iconv.h>
#include <string.h>

#define ICONV_CONST ${ICONV_CONST}

int main() {
  int result = 0;
  /* Test against AIX 5.1 bug: Failures are not distinguishable from successful
     returns.  */
  {
    iconv_t cd_utf8_to_88591 = iconv_open (\"ISO8859-1\", \"UTF-8\");
    if (cd_utf8_to_88591 != (iconv_t)(-1))
      {
        static ICONV_CONST char input[] = \"\\\\342\\\\202\\\\254\"; /* EURO SIGN */
        char buf[10];
        ICONV_CONST char *inptr = input;
        size_t inbytesleft = strlen (input);
        char *outptr = buf;
        size_t outbytesleft = sizeof (buf);
        size_t res = iconv (cd_utf8_to_88591,
                            &inptr, &inbytesleft,
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
    if (cd_ascii_to_88591 != (iconv_t)(-1))
      {
        static ICONV_CONST char input[] = \"\\\\263\";
        char buf[10];
        ICONV_CONST char *inptr = input;
        size_t inbytesleft = strlen (input);
        char *outptr = buf;
        size_t outbytesleft = sizeof (buf);
        size_t res = iconv (cd_ascii_to_88591,
                            &inptr, &inbytesleft,
                            &outptr, &outbytesleft);
        if (res == 0)
          result |= 2;
        iconv_close (cd_ascii_to_88591);
      }
  }
  /* Test against AIX 6.1..7.1 bug: Buffer overrun.  */
  {
    iconv_t cd_88591_to_utf8 = iconv_open (\"UTF-8\", \"ISO-8859-1\");
    if (cd_88591_to_utf8 != (iconv_t)(-1))
      {
        static ICONV_CONST char input[] = \"\\\\304\";
        static char buf[2] = { (char)0xDE, (char)0xAD };
        ICONV_CONST char *inptr = input;
        size_t inbytesleft = 1;
        char *outptr = buf;
        size_t outbytesleft = 1;
        size_t res = iconv (cd_88591_to_utf8,
                            &inptr, &inbytesleft,
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
    if (cd_88591_to_utf8 != (iconv_t)(-1))
      {
        static ICONV_CONST char input[] = \"\\\\304rger mit b\\\\366sen B\\\\374bchen ohne Augenma\\\\337\";
        char buf[50];
        ICONV_CONST char *inptr = input;
        size_t inbytesleft = strlen (input);
        char *outptr = buf;
        size_t outbytesleft = sizeof (buf);
        size_t res = iconv (cd_88591_to_utf8,
                            &inptr, &inbytesleft,
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
      if(NOT DEFINED ICONV_WORKS)
        # Guess value for ICONV_WORKS based on the name of the host system.
        if(NOT ICONV_FIND_QUIETLY)
          message(STATUS "Performing Test ICONV_WORKS")
        endif()
        if(CMAKE_HOST_SYSTEM_NAME STREQUAL "AIX" OR CMAKE_HOST_SYSTEM_NAME STREQUAL "HP-UX")
          set(ICONV_WORKS FALSE CACHE BOOL "Whether iconv() works on the host system (set to false for AIX and HP-UX by default).")
        else()
          set(ICONV_WORKS TRUE CACHE BOOL "Whether iconv() works on the host system (set to true by default).")
        endif()
        if(NOT ICONV_FIND_QUIETLY)
          if(ICONV_WORKS)
            message(STATUS "Performing Test ICONV_WORKS - success (guessed)")
          else()
            message(STATUS "Performing Test ICONV_WORKS - failed (guessed)")
          endif()
        endif()
      endif()
      mark_as_advanced(ICONV_WORKS)
    endif()

    # Check for the GNU "flavor" of iconv().
    check_c_source_compiles("
#include <iconv.h>

#if !(defined _LIBICONV_VERSION || (defined __GLIBC__ && !defined __UCLIBC__))
#  error \"Neither GNU libiconv nor GNU libc is installed.\"
#endif

int main() {
  return 0;
}
" HAVE_GNU_ICONV)

    # Check whether iconv() supports conversion between UTF-8 and
    # UTF-{16,32}{BE,LE}.
    if(NOT CMAKE_CROSSCOMPILING)
      check_c_source_runs("
#include <iconv.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ICONV_CONST ${ICONV_CONST}

int main ()
{
  int result = 0;
  /* Test conversion from UTF-8 to UTF-16BE with no errors.  */
  {
    static const char input[] =
      \"Japanese (\\\\346\\\\227\\\\245\\\\346\\\\234\\\\254\\\\350\\\\252\\\\236) [\\\\360\\\\235\\\\224\\\\215\\\\360\\\\235\\\\224\\\\236\\\\360\\\\235\\\\224\\\\255]\";
    static const char expected[] =
      \"\\\\000J\\\\000a\\\\000p\\\\000a\\\\000n\\\\000e\\\\000s\\\\000e\\\\000 \\\\000(\\\\145\\\\345\\\\147\\\\054\\\\212\\\\236\\\\000)\\\\000 \\\\000[\\\\330\\\\065\\\\335\\\\015\\\\330\\\\065\\\\335\\\\036\\\\330\\\\065\\\\335\\\\055\\\\000]\";
    iconv_t cd;
    cd = iconv_open (\"UTF-16BE\", \"UTF-8\");
    if (cd == (iconv_t)(-1))
      result |= 1;
    else
      {
        char buf[100];
        const char *inptr;
        size_t inbytesleft;
        char *outptr;
        size_t outbytesleft;
        size_t res;
        inptr = input;
        inbytesleft = sizeof (input) - 1;
        outptr = buf;
        outbytesleft = sizeof (buf);
        res = iconv (cd,
                     (ICONV_CONST char **) &inptr, &inbytesleft,
                     &outptr, &outbytesleft);
        if (!(res == 0 && inbytesleft == 0))
          result |= 1;
        else if (!(outptr == buf + (sizeof (expected) - 1)))
          result |= 1;
        else if (!(memcmp (buf, expected, sizeof (expected) - 1) == 0))
          result |= 1;
        else if (!(iconv_close (cd) == 0))
          result |= 1;
      }
  }
  /* Test conversion from UTF-8 to UTF-16LE with no errors.  */
  {
    static const char input[] =
      \"Japanese (\\\\346\\\\227\\\\245\\\\346\\\\234\\\\254\\\\350\\\\252\\\\236) [\\\\360\\\\235\\\\224\\\\215\\\\360\\\\235\\\\224\\\\236\\\\360\\\\235\\\\224\\\\255]\";
    static const char expected[] =
      \"J\\\\000a\\\\000p\\\\000a\\\\000n\\\\000e\\\\000s\\\\000e\\\\000 \\\\000(\\\\000\\\\345\\\\145\\\\054\\\\147\\\\236\\\\212)\\\\000 \\\\000[\\\\000\\\\065\\\\330\\\\015\\\\335\\\\065\\\\330\\\\036\\\\335\\\\065\\\\330\\\\055\\\\335]\\\\000\";
    iconv_t cd;
    cd = iconv_open (\"UTF-16LE\", \"UTF-8\");
    if (cd == (iconv_t)(-1))
      result |= 2;
    else
      {
        char buf[100];
        const char *inptr;
        size_t inbytesleft;
        char *outptr;
        size_t outbytesleft;
        size_t res;
        inptr = input;
        inbytesleft = sizeof (input) - 1;
        outptr = buf;
        outbytesleft = sizeof (buf);
        res = iconv (cd,
                     (ICONV_CONST char **) &inptr, &inbytesleft,
                     &outptr, &outbytesleft);
        if (!(res == 0 && inbytesleft == 0))
          result |= 2;
        else if (!(outptr == buf + (sizeof (expected) - 1)))
          result |= 2;
        else if (!(memcmp (buf, expected, sizeof (expected) - 1) == 0))
          result |= 2;
        else if (!(iconv_close (cd) == 0))
          result |= 2;
      }
  }
  /* Test conversion from UTF-8 to UTF-32BE with no errors.  */
  {
    static const char input[] =
      \"Japanese (\\\\346\\\\227\\\\245\\\\346\\\\234\\\\254\\\\350\\\\252\\\\236) [\\\\360\\\\235\\\\224\\\\215\\\\360\\\\235\\\\224\\\\236\\\\360\\\\235\\\\224\\\\255]\";
    static const char expected[] =
      \"\\\\000\\\\000\\\\000J\\\\000\\\\000\\\\000a\\\\000\\\\000\\\\000p\\\\000\\\\000\\\\000a\\\\000\\\\000\\\\000n\\\\000\\\\000\\\\000e\\\\000\\\\000\\\\000s\\\\000\\\\000\\\\000e\\\\000\\\\000\\\\000 \\\\000\\\\000\\\\000(\\\\000\\\\000\\\\145\\\\345\\\\000\\\\000\\\\147\\\\054\\\\000\\\\000\\\\212\\\\236\\\\000\\\\000\\\\000)\\\\000\\\\000\\\\000 \\\\000\\\\000\\\\000[\\\\000\\\\001\\\\325\\\\015\\\\000\\\\001\\\\325\\\\036\\\\000\\\\001\\\\325\\\\055\\\\000\\\\000\\\\000]\";
    iconv_t cd;
    cd = iconv_open (\"UTF-32BE\", \"UTF-8\");
    if (cd == (iconv_t)(-1))
      result |= 4;
    else
      {
        char buf[100];
        const char *inptr;
        size_t inbytesleft;
        char *outptr;
        size_t outbytesleft;
        size_t res;
        inptr = input;
        inbytesleft = sizeof (input) - 1;
        outptr = buf;
        outbytesleft = sizeof (buf);
        res = iconv (cd,
                     (ICONV_CONST char **) &inptr, &inbytesleft,
                     &outptr, &outbytesleft);
        if (!(res == 0 && inbytesleft == 0))
          result |= 4;
        else if (!(outptr == buf + (sizeof (expected) - 1)))
          result |= 4;
        else if (!(memcmp (buf, expected, sizeof (expected) - 1) == 0))
          result |= 4;
        else if (!(iconv_close (cd) == 0))
          result |= 4;
      }
  }
  /* Test conversion from UTF-8 to UTF-32LE with no errors.  */
  {
    static const char input[] =
      \"Japanese (\\\\346\\\\227\\\\245\\\\346\\\\234\\\\254\\\\350\\\\252\\\\236) [\\\\360\\\\235\\\\224\\\\215\\\\360\\\\235\\\\224\\\\236\\\\360\\\\235\\\\224\\\\255]\";
    static const char expected[] =
      \"J\\\\000\\\\000\\\\000a\\\\000\\\\000\\\\000p\\\\000\\\\000\\\\000a\\\\000\\\\000\\\\000n\\\\000\\\\000\\\\000e\\\\000\\\\000\\\\000s\\\\000\\\\000\\\\000e\\\\000\\\\000\\\\000 \\\\000\\\\000\\\\000(\\\\000\\\\000\\\\000\\\\345\\\\145\\\\000\\\\000\\\\054\\\\147\\\\000\\\\000\\\\236\\\\212\\\\000\\\\000)\\\\000\\\\000\\\\000 \\\\000\\\\000\\\\000[\\\\000\\\\000\\\\000\\\\015\\\\325\\\\001\\\\000\\\\036\\\\325\\\\001\\\\000\\\\055\\\\325\\\\001\\\\000]\\\\000\\\\000\\\\000\";
    iconv_t cd;
    cd = iconv_open (\"UTF-32LE\", \"UTF-8\");
    if (cd == (iconv_t)(-1))
      result |= 8;
    else
      {
        char buf[100];
        const char *inptr;
        size_t inbytesleft;
        char *outptr;
        size_t outbytesleft;
        size_t res;
        inptr = input;
        inbytesleft = sizeof (input) - 1;
        outptr = buf;
        outbytesleft = sizeof (buf);
        res = iconv (cd,
                     (ICONV_CONST char **) &inptr, &inbytesleft,
                     &outptr, &outbytesleft);
        if (!(res == 0 && inbytesleft == 0))
          result |= 8;
        else if (!(outptr == buf + (sizeof (expected) - 1)))
          result |= 8;
        else if (!(memcmp (buf, expected, sizeof (expected) - 1) == 0))
          result |= 8;
        else if (!(iconv_close (cd) == 0))
          result |= 8;
      }
  }
  /* Test conversion from UTF-16BE to UTF-8 with no errors.
     This test fails on NetBSD 3.0.  */
  {
    static const char input[] =
      \"\\\\000J\\\\000a\\\\000p\\\\000a\\\\000n\\\\000e\\\\000s\\\\000e\\\\000 \\\\000(\\\\145\\\\345\\\\147\\\\054\\\\212\\\\236\\\\000)\\\\000 \\\\000[\\\\330\\\\065\\\\335\\\\015\\\\330\\\\065\\\\335\\\\036\\\\330\\\\065\\\\335\\\\055\\\\000]\";
    static const char expected[] =
      \"Japanese (\\\\346\\\\227\\\\245\\\\346\\\\234\\\\254\\\\350\\\\252\\\\236) [\\\\360\\\\235\\\\224\\\\215\\\\360\\\\235\\\\224\\\\236\\\\360\\\\235\\\\224\\\\255]\";
    iconv_t cd;
    cd = iconv_open (\"UTF-8\", \"UTF-16BE\");
    if (cd == (iconv_t)(-1))
      result |= 16;
    else
      {
        char buf[100];
        const char *inptr;
        size_t inbytesleft;
        char *outptr;
        size_t outbytesleft;
        size_t res;
        inptr = input;
        inbytesleft = sizeof (input) - 1;
        outptr = buf;
        outbytesleft = sizeof (buf);
        res = iconv (cd,
                     (ICONV_CONST char **) &inptr, &inbytesleft,
                     &outptr, &outbytesleft);
        if (!(res == 0 && inbytesleft == 0))
          result |= 16;
        else if (!(outptr == buf + (sizeof (expected) - 1)))
          result |= 16;
        else if (!(memcmp (buf, expected, sizeof (expected) - 1) == 0))
          result |= 16;
        else if (!(iconv_close (cd) == 0))
          result |= 16;
      }
  }
  return result;
}
" ICONV_SUPPORTS_UNICODE)
    else()
      if(NOT DEFINED ICONV_SUPPORTS_UNICODE)
        # We know that GNU libiconv, GNU libc, and Solaris >= 9 support the
        # requisite conversions.
        # NOTE: OSF/1 5.1 has these encodings, but inserts a BOM in the "to"
        # direction.
        if(NOT ICONV_FIND_QUIETLY)
          message(STATUS "Performing Test ICONV_SUPPORTS_UNICODE")
        endif()
        if(HAVE_GNU_ICONV OR CMAKE_HOST_SYSTEM MATCHES "SunOS-(2\\.9|2\\.1[0-9])")
          set(ICONV_SUPPORTS_UNICODE 1 CACHE INTERNAL "Whether iconv() supports conversion between UTF-8 and UTF-{16,32}{BE,LE} on the host system.")
          if(NOT ICONV_FIND_QUIETLY)
            message(STATUS "Performing Test ICONV_SUPPORTS_UNICODE - success")
          endif()
        else()
          set(ICONV_SUPPORTS_UNICODE FALSE CACHE BOOL "Whether iconv() supports conversion between UTF-8 and UTF-{16,32}{BE,LE} on the host system (set to false by default).")
          if(NOT ICONV_FIND_QUIETLY)
            message(STATUS "Performing Test ICONV_SUPPORTS_UNICODE - failed (guessed)")
          endif()
        endif()
      endif()
      mark_as_advanced(ICONV_SUPPORTS_UNICODE)
    endif()
  endif()

  cmake_pop_check_state()
endif()

set(_ICONV_REQUIRED_VARS ICONV_INCLUDE_DIR ICONV_WORKS)

set(ICONV_INCLUDE_DIRS)
set(ICONV_LIBRARIES)

if(ICONV_INCLUDE_DIR)
  set(ICONV_INCLUDE_DIRS ${ICONV_INCLUDE_DIR})
endif()

if(ICONV_LIBRARY)
  set(_ICONV_REQUIRED_VARS ICONV_LIBRARY ${_ICONV_REQUIRED_VARS})
  set(ICONV_LIBRARIES ${ICONV_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ICONV
                                  REQUIRED_VARS ${_ICONV_REQUIRED_VARS})

set(HAVE_ICONV ${ICONV_FOUND})

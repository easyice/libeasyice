AC_INIT([libdvbpsi], [1.3.1])

AC_PREREQ([2.50])
AC_CONFIG_AUX_DIR([.auto])
AC_CANONICAL_SYSTEM
AC_CONFIG_SRCDIR([src/dvbpsi.c])

AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

dnl AC_CANONICAL_HOST
AC_PROG_CC
AC_STDC_HEADERS
AC_C_INLINE

AM_PROG_CC_C_O

AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

dnl store CFLAGS from user
CFLAGS_save="${CFLAGS}"

dnl add features to CFLAGS
CFLAGS_dist="-Wall -std=gnu99 -D_GNU_SOURCE"
CFLAGS_dist="${CFLAGS_dist} -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wshadow -Waggregate-return -Wmissing-prototypes -Wnested-externs -Wsign-compare"

dnl check the operating system
case "${target_os}" in
  darwin*)
    SYS=darwin
    CFLAGS_dist="${CFLAGS_dist} -no-cpp-precomp"
    ;;
  *mingw32*)
    SYS=mingw32
    CFLAGS_dist="${CFLAGS_dist} -Wno-cast-qual"
    AC_DEFINE([WIN32],[1],["Define to 1 if building for windows with mingw32"])
    ;;
   *)
    SYS=linux
    CFLAGS_dist="${CFLAGS_dist} -Wcast-qual"
   ;;
esac

dnl --enable-debug
AC_ARG_ENABLE(debug,
[  --enable-debug          Enable debug mode (default disabled)],
[case "${enableval}" in
  yes) debug=true ;;
  no)  debug=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac],[debug=false])
if test "$debug" = "true"
then
  CFLAGS_dist="${CFLAGS_dist} -Werror -ggdb3"
fi

dnl --enable-gcc-sanitize
dnl NOTE: Only use with more then 8GB of memory in your system
AC_ARG_ENABLE(gcc-sanitize,
[  --enable-gcc-sanitize   Use gcc address sanitizer (default disabled)],
[case "${enableval}" in
  yes) sanitize=true ;;
  no)  sanitize=false;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-gcc-sanitize) ;;
esac],[sanitize=false])
if test "$sanitize" = "true"
then
  CFLAGS_dist="${CFLAG_dist} -fsanitize=address -fno-omit-frame-pointer"
fi

dnl --enable-release
AC_ARG_ENABLE(release,
[  --enable-release        Enable release mode (default disabled)],
[case "${enableval}" in
  yes) release=true ;;
  no)  release=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-release) ;;
esac],[release=false])
if test "$release" != "true"; then
  CFLAGS_dist="${CFLAGS_dist} -DDVBPSI_DIST"
fi

dnl compile feature tests
CFLAGS="${CFLAGS_save} ${CFLAGS_dist}"

dnl Check for headers
AC_CHECK_HEADERS([stdbool.h stdint.h inttypes.h getopt.h strings.h sys/time.h])
dnl AC_CHECK_FUNCS([gettimeofday])

AC_CHECK_HEADERS([sys/socket.h], [ac_have_sys_socket_h=yes])
AM_CONDITIONAL(HAVE_SYS_SOCKET_H, test "${ac_have_sys_socket_h}" = "yes")

AC_CHECK_HEADERS([net/if.h], [], [],
  [
    #include <sys/types.h>
    #include <sys/socket.h>
  ])

dnl Check for variadic macros
AC_CACHE_CHECK([for variadic cpp macros],
    [ac_cv_cpp_variadic_macros],
    [AC_TRY_COMPILE(
         [#include <stdio.h>
          #define a(b,c...) printf(b,##c)],
         [a("foo");a("%s","bar");a("%s%s","baz","quux");],
         ac_cv_cpp_variadic_macros=yes,
         ac_cv_cpp_variadic_macros=no)])
if test "${ac_cv_cpp_variadic_macros}" != "no"; then
    AC_DEFINE(HAVE_VARIADIC_MACROS, 1, Support for variadic macros)
fi

dnl Check for asprintf(). Systems with asprintf() must also have vasprintf(),
dnl so it is not checked separately.
AC_CACHE_CHECK([for asprintf()],
    [ac_cv_asprintf],
    [AC_COMPILE_IFELSE([
        AC_LANG_SOURCE([[
            #include <stdio.h>
            #include <stdlib.h>
            int main(void) {
                char *text = NULL;
                int ret = asprintf(&text, "test");
                if (ret >= 0) free(text);
                return 0;
            }
        ]])],
        ac_cv_asprintf=yes,
        ac_cv_asprintf=no)])
if test "${ac_cv_asprintf}" != "no"; then
    AC_DEFINE(HAVE_ASPRINTF, 1, [Support for asprintf() and vasprintf()])
fi

dnl
dnl Generate Makefiles and other output files
dnl
AC_OUTPUT([Makefile
           src/Makefile
           examples/Makefile
           examples/dvbinfo/Makefile
           misc/Makefile
           doc/Makefile
           wince/Makefile
           libdvbpsi.pc
           libdvbpsi.spec])

echo "
libdvbpsi configuration
-----------------------
libdvbpsi version     : ${VERSION}
debug                 : ${debug}
release               : ${release}
compile flags         : ${CFLAGS}
build for             : ${SYS}
"

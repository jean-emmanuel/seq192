dnl Process this file with autoconf to produce a configure script.
AC_INIT(seq192, [0.11.0], [seq192-devel@lists.sourceforge.net])

AC_CONFIG_HEADER([src/config.h])

AM_INIT_AUTOMAKE([dist-bzip2 subdir-objects])

AC_DEFINE(_GNU_SOURCE, 1, [gnu source])

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S

dnl Do we have -Wl,--as-needed?
AC_MSG_CHECKING(if --as-needed works)
AC_ARG_ENABLE(as_needed,[ --enable-as-needed enable -Wl,--as-needed],
     [
     case "${enableval}" in
         yes) as_needed="1";;
         no) as_needed="";;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-as_needed);;
     esac
     ],[
         as_needed="unknown"
     ])
if test x"${as_needed}" = x"unknown"; then
    ac_old_ldflags="${LDFLAGS}"
    LDFLAGS="-Wl,--as-needed"
    AC_TRY_LINK(
                [],
                [],
                [as_needed="1"],
                [as_needed=""])
    LDFLAGS="${ac_old_ldflags}"
fi
if test -n "$as_needed"; then
    AC_MSG_RESULT(yes)
    LDFLAGS="${LDFLAGS} -Wl,--as-needed"
else
    AC_MSG_RESULT(no)
fi

dnl Checks for libraries.
AC_CHECK_LIB(rt, main, ,
	AC_MSG_ERROR([POSIX.1b RealTime Library Missing -lrt]))
AC_CHECK_LIB(gtkmm-3.0, _init,,
             AC_MSG_ERROR(Essential library libgtkmm-3.0 not found))
AC_CHECK_LIB(sigc-2.0, main,,
             AC_MSG_ERROR(Essential library libsigc++-2.0 not found))
AC_CHECK_LIB(asound, snd_pcm_open,,
             AC_MSG_ERROR(Essential library asound not found))

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_CHECK_FUNCS(select)

AM_PATH_ALSA(0.9.0)

PKG_CHECK_MODULES(GTKMM, gtkmm-3.0 >= 3.24.0)
AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)

PKG_CHECK_MODULES(JACK, jack >= 0.90.0)
AC_SUBST(JACK_CFLAGS)
AC_SUBST(JACK_LIBS)

AC_OUTPUT(Makefile man/Makefile src/Makefile)

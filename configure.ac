AC_PREREQ([2.69])

m4_define([claws_VERSION],
    m4_esyscmd([./get-git-version]))

AC_INIT([claws-mail],
    m4_defn([claws_VERSION]))
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([config])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([no-define])

PACKAGE=claws-mail

dnl version number
INTERFACE_AGE=0
BINARY_AGE=0
EXTRA_RELEASE=
EXTRA_GTK3_VERSION=

GIT_VERSION=m4_defn([claws_VERSION])

if test \( -z "$GIT_VERSION" \); then
	AC_MSG_ERROR([*** could not determine program version])
fi

MAJOR_VERSION=${GIT_VERSION%%.*}
MINOR_VERSION=${GIT_VERSION#*.}
MINOR_VERSION=${MINOR_VERSION%%.*}
MICRO_VERSION=${GIT_VERSION##*.}
MICRO_VERSION=${MICRO_VERSION%%-*}
EXTRA_VERSION=${GIT_VERSION#*-}
EXTRA_VERSION=${EXTRA_VERSION%%-*}

if test \( "x$EXTRA_VERSION" != "x" -a `echo -n $EXTRA_VERSION | wc -c` -lt 5 \); then
    VERSION=${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}git${EXTRA_VERSION}
else
    VERSION=${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}
    EXTRA_VERSION=0
fi

if test \( "x$EXTRA_RELEASE" != "x" \); then
    VERSION=${VERSION}${EXTRA_RELEASE}${EXTRA_GTK3_VERSION}
fi

dnl AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
dnl AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(MICRO_VERSION)
AC_SUBST(EXTRA_VERSION)
AC_SUBST(GIT_VERSION)

AC_CHECK_PROG(HAVE_GTK_ICON_CACHE, gtk-update-icon-cache, yes, no)
AM_CONDITIONAL(UPDATE_GTK_ICON_CACHE, test x"$HAVE_GTK_ICON_CACHE" = xyes)

dnl Require pkg-config
m4_ifndef([PKG_PROG_PKG_CONFIG],
    [m4_fatal([Could not locate the pkg-config autoconf macros. These
are usually located in /usr/share/aclocal/pkg.m4. If your macros
are in a different location, try setting the environment variable
ACLOCAL_FLAGS before running ./autogen.sh or autoreconf again. E.g.:
export ACLOCAL_FLAGS="-I/other/macro/dir"])
])
PKG_PROG_PKG_CONFIG

dnl libtool versioning
LT_RELEASE=$MAJOR_VERSION.$MINOR_VERSION
LT_CURRENT=`expr $MICRO_VERSION - $INTERFACE_AGE`
LT_REVISION=$INTERFACE_AGE
LT_AGE=`expr $BINARY_AGE - $INTERFACE_AGE`
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

dnl Specify a header configuration file
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_HEADERS(claws-features.h)

AM_MAINTAINER_MODE
AC_USE_SYSTEM_EXTENSIONS

dnl Checks for programs.
dnl AC_ARG_PROGRAM
AC_PROG_CC
AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_CPP
dnl AC_PROG_RANLIB
AC_PROG_LEX(noyywrap)
AC_PROG_YACC
AC_LIB_PREFIX

LT_INIT
LT_PROG_RC
LT_LANG([Windows Resource])
LT_INIT
AC_PROG_AWK

AC_SYS_LARGEFILE

LDFLAGS="$LDFLAGS -Wl,--export-dynamic"

AM_CFLAGS="-Wall"
if test $USE_MAINTAINER_MODE = yes; then
	AM_CFLAGS="$AM_CFLAGS -g"
fi

AC_SUBST(AM_CFLAGS)

dnl Checks for iconv
AM_ICONV

dnl floor and ceil are  in -lm
LIBS="$LIBS -lm"

GETTEXT_PACKAGE=claws-mail
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define text domain.])

AM_GNU_GETTEXT_VERSION([0.18])
AM_GNU_GETTEXT([external])

AC_ARG_ENABLE(gnutls,
		[  --disable-gnutls                Do not build GnuTLS support for TLS],
		    [enable_gnutls=$enableval], [enable_gnutls=yes])

AC_ARG_ENABLE(oauth2,
		[  --disable-oauth2                Do not build OAuth2 support],
		    [enable_oauth2=$enableval], [enable_oauth2=yes])

AC_ARG_ENABLE(valgrind,
		[  --disable-valgrind              Do not build valgrind support for debugging],
		[enable_valgrind=$enableval], [enable_valgrind=yes])

AC_ARG_ENABLE(tests,
				[  --enable-tests                   Build unit tests],
				[enable_tests=$enableval], [enable_tests=no])

AC_ARG_ENABLE(more-addressbook-debug,
        [  --enable-more-addressbook-debug Build with additional addressbook debug calls],
        [enable_more_addressbook_debug=$enableval], [enable_more_addressbook_debug=no])

dnl Set PACKAGE_DATA_DIR in config.h.
if test "x${datarootdir}" = 'x${prefix}/share'; then
	if test "x${prefix}" = "xNONE"; then
    		AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}",[PACKAGE_DATA_DIR])
  	else
    		AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}",[PACKAGE_DATA_DIR])
  	fi
else
	AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${datarootdir}/${PACKAGE}",[PACKAGE_DATA_DIR])
fi

AC_CHECK_LIB(xpg4, setlocale)

dnl Check for __VA_OPT__ macro
AC_CACHE_CHECK([for __VA_OPT__],
	[ac_cv_va_opt],
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <stdio.h>]], [[#define va_opt_printf(format, ...) fprintf(stderr, format __VA_OPT__(,) __VA_ARGS__)
		va_opt_printf("success\n");]])],[ac_cv_va_opt=yes],[ac_cv_va_opt=no])]
)
if test "$ac_cv_va_opt" = yes; then
	AC_DEFINE([HAVE_VA_OPT], [1], [Define if __VA_OPT__ macro works])
fi

dnl Checks for header files.
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h sys/file.h unistd.h paths.h \
		 sys/param.h sys/select.h \
		 wchar.h wctype.h locale.h netdb.h)
AC_CHECK_HEADER([execinfo.h], [AC_DEFINE(HAVE_BACKTRACE,1,[Has backtrace*() needed for retrieving stack traces])])
AC_SEARCH_LIBS(backtrace_symbols, [execinfo])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

AC_CHECK_SIZEOF(unsigned short, 2)
AC_CHECK_SIZEOF(unsigned int, 4)
AC_CHECK_SIZEOF(unsigned long, 4)

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_CHECK_FUNCS(strcasestr)

dnl check for glib
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.50 gmodule-2.0 >= 2.50 gobject-2.0 >= 2.50 gthread-2.0 >= 2.50])

GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)

AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(GDK_PIXBUF, [gdk-pixbuf-2.0 >= 2.26])

dnl GNUTLS
AC_MSG_CHECKING([whether to use GnuTLS])
AC_MSG_RESULT($enable_gnutls)
if test "x$enable_gnutls" != "xno"; then
        PKG_CHECK_MODULES(GNUTLS, gnutls >= 2.2,
        [
                AC_DEFINE(USE_GNUTLS, 1, gnutls)
                echo "Building with GnuTLS"
       ],
        [
                echo "Building without GnuTLS"
		AC_MSG_RESULT([*** GnuTLS support is recommended ])
		AC_MSG_RESULT([*** You can use --disable-gnutls if you don't need it.])
		AC_MSG_ERROR([GnuTLS not found])
        ])
        AC_SUBST(GNUTLS_LIBS)
        AC_SUBST(GNUTLS_CFLAGS)
fi

dnl RC dir (will be default at a certain point in time)
AC_ARG_WITH(config-dir,    [  --with-config-dir=RCDIR      Local configuration dir (default: .claws-mail)],
	      ac_cv_with_config_dir="$withval", ac_cv_with_config_dir="")

if test x"$ac_cv_with_config_dir" = x""; then
	ac_cv_with_config_dir=".claws-mail"
fi
AC_DEFINE_UNQUOTED(CFG_RC_DIR, "$ac_cv_with_config_dir", Configuration directory)

if ! `$PKG_CONFIG --atleast-version=3.0 gnutls`; then
	AC_MSG_ERROR([GnuTLS version at least 3.0 is required for password encryption.])
fi

AC_MSG_CHECKING([whether GnuTLS support is present for OAuth2])
if test x"$enable_gnutls" = xyes; then
	AC_MSG_RESULT([yes])
	AC_MSG_CHECKING([whether to build OAuth2 support])
	if test x"$enable_oauth2" = xyes; then
		AC_MSG_RESULT([yes])
		AC_DEFINE(USE_OAUTH2, 1, [Define if OAuth2 is to be activated.])
	else
		AC_MSG_RESULT([no])
		enable_oauth2=no
	fi
else
	AC_MSG_RESULT([no])
	enable_oauth2=no
fi

PKG_CHECK_MODULES(GTK, gtk+-3.0 >= 3.20 cairo)

AC_ARG_ENABLE(deprecated,
		[  --disable-deprecated            Disable deprecated GTK functions],
		gtkdeprecated=$enableval)
AC_MSG_CHECKING([whether to use deprecated GTK functions])
if test x"$gtkdeprecated" != xno; then
	AC_MSG_RESULT(yes)
else
	GTK_CFLAGS="$GTK_CFLAGS -DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"
	AC_MSG_RESULT(no)
fi

dnl Make sure the code does not regress to using deprecated GTK stuff...
GTK_CFLAGS="$GTK_CFLAGS -DGTK_DISABLE_SINGLE_INCLUDES -DGSEAL_ENABLE"

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES([LIBETPAN], [libetpan >= 1.9.4])
LIBETPAN_LIBS=`pkg-config --libs libetpan`
AC_SUBST(LIBETPAN_LIBS)
LIBS="$LIBS $LIBETPAN_LIBS"
LIBETPAN_CFLAGS=`pkg-config --cflags libetpan`
AC_SUBST(LIBETPAN_CFLAGS)
CPPFLAGS="$CPPFLAGS $LIBETPAN_CFLAGS"

AC_MSG_CHECKING([whether to use valgrind])
if test x$enable_valgrind = xyes; then
	AC_MSG_RESULT(yes)
	PKG_CHECK_MODULES([VALGRIND], [valgrind >= 2.4.0],
	[
		AC_DEFINE(HAVE_VALGRIND, 1, Define if you want valgrind support)
		enable_valgrind=yes
	],
	[
		AC_MSG_NOTICE([valgrind support deactivated as valgrind >= 2.4.0 was not found])
		enable_valgrind=no
	])
else
	AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(CLAWS_VALGRIND, test x"$enable_valgrind" = x"yes")

AC_MSG_CHECKING([whether to build unit tests])
if test x$enable_tests = xyes; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(BUILD_TESTS, test "x$enable_tests" = "xyes")

more_debug_output_modules=""

AC_MSG_CHECKING([whether to build addressbook with more debug calls])
if test x$enable_more_addressbook_debug = xyes; then
	more_debug_output_modules="$more_debug_output_modules AddressBook"
	AC_MSG_RESULT(yes)
	AC_DEFINE(DEBUG_ADDRBOOK, 1, [Define if you want additional addressbook debug calls])
else
	AC_MSG_RESULT(no)
fi

PKG_CHECK_MODULES(EXPAT, expat, HAVE_EXPAT=yes, HAVE_EXPAT=no)

if test x"$HAVE_EXPAT" = xno; then
	AC_CHECK_HEADER(expat.h, [expat_header=yes], [expat_header=no])
	AC_CHECK_LIB(expat, XML_ParserCreate, [expat_lib=yes], [expat_lib=no])
	if test x"$expat_header" = xyes -a x"$expat_lib"=xyes; then
		HAVE_EXPAT=yes
		EXPAT_CFLAGS=""
		EXPAT_LIBS="-lexpat"
	fi
fi

AC_SUBST(EXPAT_CFLAGS)
AC_SUBST(EXPAT_LIBS)

PKG_CHECK_MODULES(CAIRO, cairo >= 1.12.0, HAVE_CAIRO=yes, HAVE_CAIRO=no)
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

PKG_CHECK_MODULES(FONTCONFIG, fontconfig, HAVE_FONTCONFIG=yes, HAVE_FONTCONFIG=no)
AC_SUBST(FONTCONFIG_CFLAGS)
AC_SUBST(FONTCONFIG_LIBS)

AC_CONFIG_FILES([
Makefile
src/common/version.h
src/Makefile
src/common/Makefile
src/common/tests/Makefile
src/gtk/Makefile
src/etpan/Makefile
src/tests/Makefile
claws-mail.pc
])
AC_OUTPUT

echo "$PACKAGE $VERSION"
echo ""
echo "gnuTLS               : $enable_gnutls"
echo "OAuth2               : $enable_oauth2"
echo "iconv                : $am_cv_func_iconv"
echo "Unit tests           : $enable_tests"
echo "Valgrind support     : $enable_valgrind"

if test -n "$more_debug_output_modules"; then
	echo "Extra debug output enabled for:"
	for module in $more_debug_output_modules; do
		echo "            - $module"
	done
fi

echo ""
echo "The binary will be installed in $prefix/bin"
echo ""
echo "Configuration directory: $ac_cv_with_config_dir"
echo ""
echo "Configure finished, type 'make' to build."

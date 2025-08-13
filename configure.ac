
AC_PREREQ([2.69])

m4_define([claws_VERSION],
    m4_esyscmd([./get-git-version]))

AC_INIT([claws-mail],
    m4_defn([claws_VERSION]))
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
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
AU_DEFUN([AC_LIBTOOL_WIN32_DLL],
[AC_REQUIRE(AC_CANONICAL_HOST)_LT_SET_OPTION([LT_INIT],[win32-dll])
m4_warn([obsolete],[AC_LIBTOOL_WIN32_DLL: Remove this warning and the call to _LT_SET_OPTION when you
put the 'win32-dll' option into LT_INIT's first parameter.])
])

LT_INIT
LT_PROG_RC
LT_LANG([Windows Resource])
LT_INIT
AC_PROG_AWK

dnl AC_PROG_CXX will set CXX=g++ even if it finds no useable C++
dnl compiler, so we have to check whether the program named by
dnl CXX exists.
AC_PROG_CXX
AC_PATH_PROG(REAL_CXX, $CXX)
HAVE_CXX=no
if test -n "$REAL_CXX"; then
	HAVE_CXX=yes
fi

AC_SYS_LARGEFILE

dnl Copied from the official gtk+-2 configure.in
AC_MSG_CHECKING([for host platform])
case "$host" in
  *-*-mingw*|*-*-cygwin*)
    platform_win32=yes
    LDFLAGS="$LDFLAGS -mwindows -Wl,--export-all-symbols"
    ;;
	*-apple-*)
		platform_osx=yes
		LDFLAGS="$LDFLAGS -Wl,-export_dynamic"
		;;
  *)
    platform_win32=no
		platform_osx=no
		LDFLAGS="$LDFLAGS -Wl,--export-dynamic"
    ;;
esac
AM_CONDITIONAL(PLATFORM_WIN32, test x"$platform_win32" = x"yes")
AM_CONDITIONAL(PLATFORM_OSX, test x"$platform_osx" = x"yes")
AC_MSG_RESULT([$host])

AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    ;;
  *)
    os_win32=no
    ;;
esac
AC_MSG_RESULT([$os_win32])
AM_CONDITIONAL(OS_WIN32, test x"$os_win32" = x"yes")

AC_MSG_CHECKING([for Cygwin])
case "$host" in
  *-*-cygwin*)
    env_cygwin=yes
    ;;
  *)
    env_cygwin=no
    ;;
esac
AC_MSG_RESULT([$env_cygwin])
AM_CONDITIONAL(CYGWIN, test x"$env_cygwin" = x"yes")

AC_MSG_CHECKING([for time_t format specifier])
_gcc_cflags_save=$CFLAGS
CFLAGS="-Wformat -Werror"
AC_COMPILE_IFELSE([
	AC_LANG_PROGRAM([[
		#include <stdio.h>
		#include <time.h>
		]],[[printf("%lld", time(NULL));]])],
	[time_t_long_long=yes],
	[time_t_long_long=no])
CFLAGS=$_gcc_cflags_save

if test x"$time_t_long_long" = xyes; then
	time_t_fmt=lld
else
	time_t_fmt=ld
fi
AC_MSG_RESULT([$time_t_fmt])
AC_DEFINE_UNQUOTED([CM_TIME_FORMAT], ["$time_t_fmt"],
	[Define printf format specifier for time_t])

AM_CFLAGS="-Wall"
if test $USE_MAINTAINER_MODE = yes; then
	AM_CFLAGS="$AM_CFLAGS -g"
fi

case "$target" in
*-darwin*)
	AM_CFLAGS="$AM_CFLAGS -fno-common"
	;;
*-*-mingw*)
	AM_CFLAGS="$AM_CFLAGS -mms-bitfields"
	LIBS="$LIBS -lws2_32 -lregex"
	;;
*-*-solaris*)
	AM_CFLAGS="$AM_CFLAGS -std=gnu99"
	AC_DEFINE([SOLARIS], [], [Target is Solaris])
	;;
esac

AC_SUBST(AM_CFLAGS)

dnl Checks for iconv
AM_ICONV

dnl floor and ceil are  in -lm
LIBS="$LIBS -lm"

dnl for gettext
GETTEXT_PACKAGE=claws-mail
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define text domain.])

AM_GNU_GETTEXT_VERSION([0.18])
AM_GNU_GETTEXT([external])

AC_ARG_ENABLE(manual,
		[  --disable-manual                Do not build user manual],
		[enable_manual=$enableval], [enable_manual=yes])

AC_ARG_ENABLE(gnutls,
		[  --disable-gnutls                Do not build GnuTLS support for TLS],
		    [enable_gnutls=$enableval], [enable_gnutls=yes])

AC_ARG_ENABLE(oauth2,
		[  --disable-oauth2                Do not build OAuth2 support],
		    [enable_oauth2=$enableval], [enable_oauth2=yes])

AC_ARG_ENABLE(enchant,
		[  --disable-enchant               Do not build Enchant support for spell-checking],
		[enable_enchant=$enableval], [enable_enchant=yes])

AC_ARG_ENABLE(startup-notification,
		[  --disable-startup-notification  Do not startup notification support],
		[enable_startup_notification=$enableval], [enable_startup_notification=yes])

AC_ARG_ENABLE(libetpan,
		[  --disable-libetpan              Do not build libetpan support for IMAP4/NNTP],
		[enable_libetpan=$enableval], [enable_libetpan=yes])

AC_ARG_ENABLE(valgrind,
		[  --disable-valgrind              Do not build valgrind support for debugging],
		[enable_valgrind=$enableval], [enable_valgrind=yes])

AC_ARG_ENABLE(svg,
        [  --disable-svg                   Do not build SVG support],
        [enable_svg=$enableval], [enable_svg=yes])

AC_ARG_ENABLE(tests,
				[  --enable-tests                   Build unit tests],
				[enable_tests=$enableval], [enable_tests=no])

AC_ARG_ENABLE(more-addressbook-debug,
        [  --enable-more-addressbook-debug Build with additional addressbook debug calls],
        [enable_more_addressbook_debug=$enableval], [enable_more_addressbook_debug=no])

AC_ARG_ENABLE(timing-debug,
        [  --enable-timing-debug     Build with timing debug calls],
        [enable_timing_debug=$enableval], [enable_timing_debug=no])

manualdir='${docdir}/manual'
AC_ARG_WITH(manualdir,
	[  --with-manualdir=DIR    Manual directory],
	[manualdir="$withval"])
AC_SUBST(manualdir)

dnl ******************************
dnl ** Check for required tools **
dnl ** to build manuals         **
dnl ******************************

AC_PATH_PROG(DOCBOOK2HTML, docbook2html)
AC_PATH_PROG(DOCBOOK2TXT, docbook2txt)
AC_PATH_PROG(DOCBOOK2PS, docbook2ps)
AC_PATH_PROG(DOCBOOK2PDF, docbook2pdf)

AM_CONDITIONAL(MANUAL_HTML, test -n "$DOCBOOK2HTML")
AM_CONDITIONAL(MANUAL_TXT, test -n "$DOCBOOK2TXT")
AM_CONDITIONAL(MANUAL_PDF, test -n "$DOCBOOK2PDF")
AM_CONDITIONAL(MANUAL_PS, test -n "$DOCBOOK2PS")

if test x"$enable_manual" = x"yes"; then
    if test -n "$DOCBOOK2TXT" -o -n "$DOCBOOK2HTML" \
	-o -n "$DOCBOOK2PS" -o -n "$DOCBOOK2PDF"; then
	    enable_manual=yes
	else
	    enable_manual=no
    fi
fi

AM_CONDITIONAL(BUILD_MANUAL, test x"$enable_manual" = xyes)

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
AC_CHECK_FUNCS(fchmod flock lockf strcasestr)

dnl *****************
dnl ** common code **
dnl *****************

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

PKG_CHECK_MODULES(NETTLE, nettle)
AC_SUBST(NETTLE_LIBS)
AC_SUBST(NETTLE_CFLAGS)

AC_ARG_WITH(passcrypt-key, [  --with-passcrypt-key=KEY     Key used to encode passwords (8 byte string)],
	    with_passcrypt_key="$withval", with_passcrypt_key="passkey0")
AC_SUBST(PASSCRYPT_KEY, $with_passcrypt_key)

dnl RC dir (will be default at a certain point in time)
AC_ARG_WITH(config-dir,    [  --with-config-dir=RCDIR      Local configuration dir (default: .claws-mail)],
	      ac_cv_with_config_dir="$withval", ac_cv_with_config_dir="")

dnl Set correct default value based on platform
if test x"$ac_cv_with_config_dir" = x""; then
	if test x"$platform_win32" = xyes; then
		ac_cv_with_config_dir="Claws-mail"
	else
		ac_cv_with_config_dir=".claws-mail"
	fi
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

dnl ************************
dnl ** GTK user interface **
dnl ************************

dnl Checks for GTK
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

dnl enchant is used for spell checking
AC_MSG_CHECKING([whether to use enchant])
AC_MSG_RESULT($enable_enchant)
if test $enable_enchant = yes; then
		PKG_CHECK_MODULES(ENCHANT, enchant-2 >= 2.0.0,
		[
			AC_DEFINE(USE_ENCHANT, 1, enchant-2)
			echo "Building with enchant-2"
			enable_enchant=yes
		],
		[
			echo "Building without enchant-notification"
			enable_enchant=no
		])
	AC_SUBST(ENCHANT_CFLAGS)
	AC_SUBST(ENCHANT_LIBS)
fi

dnl
dnl Check whether we need to pass -lresolv
dnl We know that we don't need it for W32.
dnl
if test x$os_win32 = xno; then
  t_oldLibs="$LIBS"
  LIBS="$LIBS"
  ac_cv_var__res_options=no
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
 	     #include <sys/socket.h>
	     #include <netinet/in.h>
 	     #include <arpa/nameser.h>
	     #include <resolv.h>]], [[_res.options = RES_INIT;]])],[ac_cv_var__res_options=yes],[]);
  if test "$ac_cv_var__res_options" != "yes"; then
	LIBRESOLV="-lresolv"
  fi
  LIBS="$t_oldLibs"

  if test "x$LIBRESOLV" = "x"; then
	AC_CHECK_LIB(resolv, res_query, [LIBRESOLV=-lresolv])
	LIBS="$t_oldLibs"
  fi
fi
AC_SUBST(LIBRESOLV)

LIBS="$LIBS $LIBRESOLV"

dnl #######################################################################
dnl # Check for startup notification
dnl #######################################################################
if test "x$enable_startup_notification" = "xyes"; then
	PKG_CHECK_MODULES(STARTUP_NOTIFICATION, pkg_info libstartup-notification-1.0 >= 0.5,
	[
		AC_DEFINE(HAVE_STARTUP_NOTIFICATION, 1, [Define if we're using libstartup-notification.])
		echo "Building with libstartup-notification"
		enable_startup_notification=yes
	],
	[
		echo "Building without libstartup-notification"
		enable_startup_notification=no
	])

	AC_SUBST(STARTUP_NOTIFICATION_CFLAGS)
	AC_SUBST(STARTUP_NOTIFICATION_LIBS)
fi

dnl Libetpan
AC_MSG_CHECKING([whether to use libetpan])
if test x"$enable_libetpan" = xyes; then
    AC_MSG_RESULT(yes)

    libetpan_config=no
    libetpan_result=no
    libetpan_versiontype=0

    # since 1.9.4, libetpan uses pkg-config
    PKG_CHECK_MODULES([LIBETPAN], [libetpan >= 1.9.4],
    [
        LIBETPAN_VERSION=`pkg-config --modversion libetpan | $AWK -F. '{printf "%d", ($1 * 10000) + ($2 * 100) + $3}'`
        libetpan_config=yes
    ],
    [
        # before ws libetpan 1.9.4, libetpan uses its own libetpan-config script
        AC_PATH_PROG(libetpanconfig, [libetpan-config])
        if test "x$libetpanconfig" != "x"; then
            LIBETPAN_CFLAGS="`$libetpanconfig --cflags`"
            LIBETPAN_LIBS="`$libetpanconfig --libs`"
            # support libetpan version like x.x and x.x.x
            libetpan_versiontype=`$libetpanconfig --version | tr -dc . | wc -c`
            if test $libetpan_versiontype -eq 1; then
                LIBETPAN_VERSION=`$libetpanconfig --version | $AWK -F. '{printf "%d", ($1 * 100) + $2}'`
            else
                LIBETPAN_VERSION=`$libetpanconfig --version | $AWK -F. '{printf "%d", ($1 * 10000) + ($2 * 100) + $3}'`
            fi
            libetpan_config=yes
        fi
    ])
    if test "x$libetpan_config" = "xyes"; then
        libetpan_save_CPPFLAGS=$CPPFLAGS
        CPPFLAGS="$CPPFLAGS $LIBETPAN_CFLAGS"
        AC_CHECK_HEADER(libetpan/libetpan.h, [libetpan_result=yes])
        if test "x$libetpan_result" = "xyes"; then
            AC_MSG_CHECKING([whether libetpan-config hints compiles and links fine])
            libetpan_save_LIBS=$LIBS
            LIBS="$LIBS $LIBETPAN_LIBS"
            AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <libetpan/dbstorage.h>]], [[db_mailstorage_init(NULL, NULL);]])],[libetpan_result=yes],[libetpan_result=no])
            LIBS=$libetpan_save_LIBS
            AC_MSG_RESULT([$libetpan_result])
        fi
        CPPFLAGS=$libetpan_save_CPPFLAGS
    fi
    if test "x$libetpan_result" = "xyes"; then
        if test $libetpan_versiontype -eq 1; then
            if test "$LIBETPAN_VERSION" -lt "57"; then
                AC_MSG_RESULT([*** Claws Mail requires libetpan 0.57 or newer. See http://www.etpan.org/])
                AC_MSG_RESULT([*** You can use --disable-libetpan if you don't need IMAP4 and/or NNTP support.])
                AC_MSG_ERROR([libetpan 0.57 not found])
            fi
        fi
        AC_SUBST(LIBETPAN_CFLAGS)
        AC_SUBST(LIBETPAN_LIBS)
        AC_DEFINE(HAVE_LIBETPAN, 1, Define if you want IMAP and/or NNTP support.)
    else
        AC_MSG_RESULT([*** Claws Mail requires libetpan 0.57 or newer. See http://www.etpan.org/ ])
        AC_MSG_RESULT([*** You can use --disable-libetpan if you don't need IMAP4 and/or NNTP support.])
        AC_MSG_ERROR([libetpan 0.57 not found])
    fi
else
    AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(CLAWS_LIBETPAN, test "x$libetpan_result" = "xyes")

dnl librsvg
AC_MSG_CHECKING([whether to use librsvg])
if test x"$enable_svg" = xyes; then
	AC_MSG_RESULT(yes)
    PKG_CHECK_MODULES([SVG], [librsvg-2.0 >= 2.39.0 cairo >= 1.0.0],
    [
        AC_SUBST(SVG_CFLAGS)
        AC_SUBST(SVG_LIBS)
		AC_DEFINE(HAVE_SVG, 1, [Define if librsvg2 is available for SVG support])
		enable_svg=yes
	],
	[
		AC_MSG_NOTICE([SVG support deactivated as librsvg2 >= 2.39.0 was not found])
		enable_svg=no
	])
else
	AC_MSG_RESULT(no)
fi

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

AC_MSG_CHECKING([whether to build with timing debug calls])
if test x$enable_timing_debug = xyes; then
	more_debug_output_modules="$more_debug_output_modules timing"
	AC_MSG_RESULT(yes)
	AC_DEFINE(DEBUG_TIMING, 1, [Define if you want timing debug calls])
else
	AC_MSG_RESULT(no)
fi

dnl expat **********************************************************************
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

dnl libarchive *****************************************************************
PKG_CHECK_MODULES(LIBARCHIVE, libarchive, HAVE_ARCHIVE=yes, HAVE_ARCHIVE=no)
AC_SUBST(ARCHIVE_LIBS)
AC_SUBST(ARCHIVE_CFLAGS)
AC_CHECK_LIB([archive], [archive_read_new],
		       ARCHIVE_LIBS=-larchive
		       HAVE_ARCHIVE=yes
		       AC_SUBST(ARCHIVE_LIBS,$ARCHIVE_CFLAGS),
		       HAVE_ARCHIVE=no
		       )

dnl cairo **********************************************************************
PKG_CHECK_MODULES(CAIRO, cairo >= 1.12.0, HAVE_CAIRO=yes, HAVE_CAIRO=no)
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

dnl fontconfig *****************************************************************
PKG_CHECK_MODULES(FONTCONFIG, fontconfig, HAVE_FONTCONFIG=yes, HAVE_FONTCONFIG=no)
AC_SUBST(FONTCONFIG_CFLAGS)
AC_SUBST(FONTCONFIG_LIBS)

dnl libical ********************************************************************
PKG_CHECK_MODULES(LIBICAL, libical >= 2.0, HAVE_LIBICAL=yes, HAVE_LIBICAL=no)
AC_SUBST(LIBICAL_CFLAGS)
AC_SUBST(LIBICAL_LIBS)

dnl Gpgme **********************************************************************
PKG_CHECK_MODULES(GPGME, [gpgme >= 1.1.1], HAVE_GPGME_PKGCONFIG=yes, HAVE_GPGME_PKGCONFIG=no)
if test x"$HAVE_GPGME_PKGCONFIG" = xyes; then
    PKG_CHECK_MODULES(LIBGPG_ERROR, [gpg-error])
else
    AM_PATH_GPGME(1.1.1, HAVE_GPGME_CONFIG=yes, HAVE_GPGME_CONFIG=no)
fi

if test x"$HAVE_GPGME_PKGCONFIG" = xyes -o x"$HAVE_GPGME_CONFIG" = xyes; then
	AC_DEFINE(USE_GPGME, 1, Define if you use GPGME to support OpenPGP.)
	HAVE_GPGME=yes
else
	HAVE_GPGME=no
fi

dnl libnotify ******************************************************************
PKG_CHECK_MODULES(libnotify, libnotify >= 0.4.3, HAVE_LIBNOTIFY=yes, HAVE_LIBNOTIFY=no)
if test x"$HAVE_LIBNOTIFY" = xyes; then
	AC_DEFINE(HAVE_LIBNOTIFY, 1, [Define if libnotify support is enabled])
fi
AC_SUBST(libnotify_CFLAGS)
AC_SUBST(libnotify_LIBS)

dnl libcanberra-gtk3 ************************************************************
PKG_CHECK_MODULES(libcanberra_gtk3, libcanberra-gtk3, HAVE_LIBCANBERRA_GTK=yes, HAVE_LIBCANBERRA_GTK=no)
if test x"$HAVE_LIBCANBERRA_GTK" = xyes; then
	AC_DEFINE(HAVE_LIBCANBERRA_GTK, 1, [Define if libcanberra-gtk3 support is enabled])
fi
AC_SUBST(libcanberra_gtk3_CFLAGS)
AC_SUBST(libcanberra_gtk3_LIBS)

AC_CONFIG_FILES([
Makefile
po/Makefile.in
src/common/version.h
src/Makefile
src/w32-resource.rc
src/common/Makefile
src/common/passcrypt.h
src/common/tests/Makefile
src/gtk/Makefile
src/etpan/Makefile
src/tests/Makefile
doc/Makefile
doc/man/Makefile
tools/Makefile
config/Makefile
manual/Makefile
claws-mail.pc
])
AC_OUTPUT

dnl Output the configuration summary
echo ""
echo "$PACKAGE $VERSION"
echo ""
echo "gnuTLS               : $enable_gnutls"
echo "OAuth2               : $enable_oauth2"
echo "iconv                : $am_cv_func_iconv"
echo "enchant              : $enable_enchant"
echo "IMAP4 (libetpan)     : $enable_libetpan"
echo "NNTP (libetpan)      : $enable_libetpan"
echo "Manual               : $enable_manual"
echo "SVG support          : $enable_svg"
echo "Unit tests           : $enable_tests"
echo "Startup notification : $enable_startup_notification"
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

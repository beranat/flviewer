dnl ------------------------------------
dnl -   flViewer autoconf script
dnl ------------------------------------
AC_PREREQ([2.69])
AC_COPYRIGHT([Copyright 2016 flViewer authors])
AC_INIT([flViewer], [1.0.0], [madrat-@users.noreply.github.com])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR(autotools)
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_HEADERS([config.h])
AM_GNU_GETTEXT_VERSION([0.19])

AC_GNU_SOURCE

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl Automake
AM_INIT_AUTOMAKE

DATE=$(date +"%Y %B %d")
AC_SUBST(DATE)

AM_CONDITIONAL(HAVE_LD_VERSION_SCRIPT, false)

dnl ------------------------------------
dnl    Dynamic loadable modules and libtool
AC_ARG_ENABLE([plugins],
	AS_HELP_STRING([--disable-plugins], [Enable plugins support (default enabled)]),
	[],
	[enable_plugins=yes]
)
AM_CONDITIONAL([SUPPORT_PLUGINS], [test "x${enable_plugins}" != "xno"])

AM_COND_IF([SUPPORT_PLUGINS],
	[
		AC_ENABLE_SHARED
		AC_DISABLE_STATIC

		AC_LIBLTDL_INSTALLABLE
dnl		LT_CONFIG_LTDL_DIR([libltdl])
dnl		LT_INIT([dlopen])
dnl		LTDL_INIT([subproject])

		AC_PROG_LIBTOOL

		AC_CHECK_HEADERS([ltdl.h], [],  AC_MSG_ERROR([libtool-ltdl-devel is not found]))
		AC_CHECK_LIB(ltdl, [lt_dlopen],
			[LTDL_LIBS=-lltdl],
			AC_MSG_ERROR([libtool-ltdl is not found]))
		AC_SUBST(LTDL_LIBS)
		PLUGIN_DIR=`echo $libdir/flviewer`
		AC_SUBST(PLUGIN_DIR)
		AC_DEFINE(SUPPORT_PLUGINS, [], [Support dynamic plugins])
		AC_CONFIG_FILES([plugins/Makefile])

		plugins=""
	]
)

# gettext
AC_ARG_ENABLE([i18n],
    AS_HELP_STRING([--disable-i18n], [Disable internalization (default enabled)]),
    [],
	[enable_i18n="yes"]
)

AM_CONDITIONAL([SUPPORT_I18N], [test "x${enable_in18n}" != "xno"])
AM_COND_IF([SUPPORT_I18N],
	[
		AM_PO_SUBDIRS
		AM_GNU_GETTEXT([external], [need-ngettext])
		AC_PATH_PROG(GETTEXT_PATH, gettext, no)
		AS_IF([test "$GETTEXT_PATH" == "no"], [AC_MSG_ERROR([gettext is not found])])
		AC_DEFINE(SUPPORT_I18N, [], [Internationalization (i18n) enabled])
	]
)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_RANLIB
AC_PROG_INSTALL

# Checks for headers.
AC_CHECK_HEADERS([string.h unistd.h stdio.h math.h])
AC_CHECK_HEADERS([assert.h])
AC_CHECK_HEADERS([dirent.h sys/stat.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_LIB(m, pow,[LIBM="-lm"], [AC_MSG_ERROR([pow not available])])
AC_SUBST(LIBM)

dnl ------------------------------------
#   fltk library and headers
AC_PATH_PROG(FLTK_CONFIG, fltk-config, no)
AS_IF([test "$FLTK_CONFIG" == "no"], [AC_MSG_ERROR([fltk-config is not found])])

AC_MSG_CHECKING([for FLTK 1.3])
FLTK_VERSION=`${FLTK_CONFIG} --api-version`
AS_IF([test ${FLTK_VERSION} != "1.3"],
	[
		AC_MSG_RESULT([no])
		AC_MSG_ERROR([FLTK 1.3 not found (available $FLTK_VERSION)])
	]
)

AC_MSG_RESULT([yes])
FLTKCFLAGS=`${FLTK_CONFIG} --cflags`
FLTKCXXFLAGS=`${FLTK_CONFIG} --cxxflags`
FLTKLIBS=`${FLTK_CONFIG} --ldflags`
FLTKLIBS_IMAGES=`${FLTK_CONFIG} --use-images --ldflags`
AC_SUBST(FLTKCFLAGS)
AC_SUBST(FLTKCXXFLAGS)
AC_SUBST(FLTKLIBS)
AC_SUBST(FLTKLIBS_IMAGES)

dnl ------------------------------------
# Standart plugins

# poppler PDF
AC_ARG_ENABLE([poppler_pdf],
    AS_HELP_STRING([--disable-poppler_pdf], [Build Poppler PDF plugin (default enabled)]),
	[],
	[enable_poppler_pdf="yes"]
)

AM_CONDITIONAL([BUILD_POPPLER_PDF], [test "x${enable_poppler_pdf}" != "xno" -a -z "$SUPPORT_PLUGINS_TRUE"])
AM_COND_IF([BUILD_POPPLER_PDF],
	[
		PKG_CHECK_MODULES([POPPLER_CPP], [poppler-cpp], [],
			AC_MSG_ERROR([poppler-cpp is not found]))
		AC_CONFIG_FILES([plugins/poppler_pdf/Makefile])
		AM_COND_IF([SUPPORT_I18N],	[AC_CONFIG_FILES([plugins/poppler_pdf/po/Makefile])])
		plugins="${plugins} fvp_poppler_pdf"
	]
)

dnl mupdf plugin needs a mupdf
AC_ARG_WITH([mupdf],
    AS_HELP_STRING([--with-mupdf], [Build mupdf plugin and specify path to source]),
	[with_mupdf="$withval"],
	[with_mupdf="no"]
)

AM_CONDITIONAL([BUILD_MUPDF], [test "x${with_mupdf}" != "xno" -a -z "$SUPPORT_PLUGINS_TRUE"])
AM_COND_IF([BUILD_MUPDF],
    [
        AC_CONFIG_FILES([plugins/mupdf/Makefile])
		MUPDF_DIR="$with_mupdf"
		AC_SUBST(MUPDF_DIR)
        AM_COND_IF([SUPPORT_I18N],  [AC_CONFIG_FILES([plugins/mupdf/po/Makefile])])
		plugins="${plugins} fvp_mupdf"
    ]
)


dnl libTIFF
AC_ARG_ENABLE([libtiff],
    AS_HELP_STRING([--disable-libtiff], [Build TIFF plugin (default enabled)]),
	[],
	[enable_libtiff="yes"]
)
AM_CONDITIONAL([BUILD_LIBTIFF], [test "x${enable_libtiff}" != "xno" -a -z "$SUPPORT_PLUGINS_TRUE" ])
AM_COND_IF([BUILD_LIBTIFF],
	[
		PKG_CHECK_MODULES([LIBTIFF], [libtiff-4], [],
			PKG_CHECK_MODULES([LIBTIFF], [libtiff], [],
				AC_MSG_ERROR([libtiff-devel is not found])))
		AC_CONFIG_FILES([plugins/libtiff/Makefile])
		plugins="${plugins} fvp_libtiff"
	]
)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/flviewer-plugin.pc])
AC_CONFIG_FILES([doc/flviewer.md])
AC_CONFIG_FILES([doc/Makefile])

AM_COND_IF([SUPPORT_I18N],
	[
		AC_CONFIG_FILES([po/Makefile.in])
	]
)

AC_CONFIG_FILES([share/Makefile])
AC_CONFIG_FILES([share/flviewer.desktop.in])
AC_OUTPUT

dnl show summory
AC_MSG_RESULT([
flViewer configuration
----------------------------------------
Version            : ${VERSION}
Localization       : ${enable_i18n}
Plug-ins suppport  : ${enable_plugins}
Plug-ins           :${plugins}
])

pkglib_LTLIBRARIES=fvp_libtiff.la

AM_CPPFLAGS=-I$(top_srcdir)/include $(COMMON_CFLAGS) $(FLTKCXXFLAGS) -DPLUGIN_DIR=@PLUGIN_DIR@
AM_LDFLAGS =-avoid-version -module -shared -export-dynamic

fvp_libtiff_la_SOURCES = TiffImage.cpp TiffImage.h
fvp_libtiff_la_CPPFLAGS=-I$(top_srcdir)/include @LIBTIFF_CFLAGS@ @FLTKCXXFLAGS@ -DFLVIEWER_PLUGIN -DPLUGIN_DIR=\"$(PLUGIN_DIR)\" -DLOCALEDIR=\"$(localedir)\"
fvp_libtiff_la_LIBADD = @LIBTIFF_LIBS@ @FLTKLIBS@

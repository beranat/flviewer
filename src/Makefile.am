bin_PROGRAMS=flviewer

flviewer_SOURCES=main.cpp \
				DirImage.cpp DirImage.h \
				MainWindow.cpp MainWindow.h

flviewer_LDADD=@FLTKLIBS@
flviewer_CPPFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/share/icons -DLOCALEDIR=\"$(localedir)\"
flviewer_CXXFLAGS=@FLTKCXXFLAGS@
flviewer_CFLAGS=@FLTKCFLAGS@

if SUPPORT_PLUGINS
headersdir=$(includedir)/@PACKAGE@
headers_HEADERS=$(top_srcdir)/include/*

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = flviewer-plugin.pc

flviewer_SOURCES+=Plugin.cpp
flviewer_LDADD+=@LTDL_LIBS@
flviewer_CPPFLAGS+=-DPLUGIN_DIR=\"$(PLUGIN_DIR)\"
endif


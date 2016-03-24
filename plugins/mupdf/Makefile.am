if SUPPORT_I18N
SUBDIRS=po
endif

mupdf_dir=@MUPDF_DIR@
pkglib_LTLIBRARIES=fvp_mupdf.la

AM_LDFLAGS = -avoid-version -module -fPIC -shared -export-dynamic

fvp_mupdf_la_SOURCES = mupdf.cpp mupdf.h
fvp_mupdf_la_LIBADD = @FLTKLIBS@ -L"$(mupdf_dir)/build/release" -lmupdf -lfreetype -ljbig2dec -ljpeg -lmujs -lopenjpeg -lz -lcrypto -lm
fvp_mupdf_la_CPPFLAGS= $(COMMON_CFLAGS) -I$(top_srcdir)/include $(FLTKCXXFLAGS) -DFLVIEWER_PLUGIN -DPLUGIN_DIR=\"$(PLUGIN_DIR)\" -DLOCALEDIR=\"$(localedir)\" -I"$(mupdf_dir)/include" -fPIC -shared


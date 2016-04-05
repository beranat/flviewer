if SUPPORT_I18N
SUBDIRS=po
endif

mupdf_dir=@MUPDF_DIR@
mupdf_lib=$(mupdf_dir)/build/release
pkglib_LTLIBRARIES=fvp_mupdf.la

fvp_mupdf_la_LDFLAGS = -avoid-version -module -fPIC -shared

if HAVE_LD_VERSION_SCRIPT
fvp_mupdf_la_LDFLAGS += -Wl,--version-script=mupdf.map
else
fvp_mupdf_la_LDFLAGS += -export-symbols-regex '^(module|_fini|_init)$'
endif

fvp_mupdf_la_SOURCES = mupdf.cpp mupdf.h
fvp_mupdf_la_LIBADD = @FLTKLIBS@ $(mupdf_lib)/libmupdf.a $(mupdf_lib)/libjbig2dec.a $(mupdf_lib)/libjpeg.a $(mupdf_lib)/libopenjpeg.a $(mupdf_lib)/libmujs.a -lfreetype -lz -lcrypto -lm
fvp_mupdf_la_CPPFLAGS= $(COMMON_CFLAGS) -I$(top_srcdir)/include $(FLTKCXXFLAGS) -DFLVIEWER_PLUGIN -DPLUGIN_DIR=\"$(PLUGIN_DIR)\" -DLOCALEDIR=\"$(localedir)\" -I"$(mupdf_dir)/include" -fPIC -shared


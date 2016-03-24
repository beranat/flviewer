if SUPPORT_I18N
SUBDIRS=po
endif

pkglib_LTLIBRARIES=fvp_poppler_pdf.la

AM_LDFLAGS = -avoid-version -module -shared -export-dynamic

fvp_poppler_pdf_la_SOURCES = PdfImage.cpp PdfImage.h
fvp_poppler_pdf_la_LIBADD = @POPPLER_CPP_LIBS@ @FLTKLIBS@
fvp_poppler_pdf_la_CPPFLAGS= $(COMMON_CFLAGS) -I$(top_srcdir)/include $(FLTKCXXFLAGS) -DFLVIEWER_PLUGIN -DPLUGIN_DIR=\"$(PLUGIN_DIR)\" -DLOCALEDIR=\"$(localedir)\"  @POPPLER_CPP_CFLAGS@

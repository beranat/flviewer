# DISABLE mupdf plugin
%bcond_without mupdf

%define version 1.0.0
%define patch   0

Name:		flviewer
Version:	%{version}
Release:	%{patch}%{?dist}
Summary:	A small and fast image viewer

License:	GPLv3+
Group:		Amusements/Graphics
Packager:	Anatoly L. Berenblit
Vendor: 	Anatoly L. Berenblit
URL: 		https://github.com/madrat-/flviewer/releases/tag/%{version}
Source0:	https://github.com/madrat-/flviewer/archive/%{version}.tar.bz2
Source1:	http://mupdf.com/downloads/mupdf-1.8-source.tar.gz

#Patch10: ... for compatibility with another systems, don't forget ++patch

BuildRequires: gcc-c++
Buildrequires: autoconf, automake, libtool
BuildRequires: fltk-devel >= 1.3
BuildRequires: poppler-cpp-devel libtiff-devel

Requires:  	fltk >= 1.3

%description
A universal document viewer combines the small size and acceptable functionalities with the versatility of supporting different kind of images, which may be extended by plugins.

%package plugin-devel
Summary: Header files for plugins development
Group: Development/Libraries
Requires: %{name} = %{version}

%description plugin-devel
Plugins (Extensions) development packages. It includes all necessary headers.

%package l10n
Summary: Languages support for %{name}
Group: Graphics/Internationalization
BuildArch: noarch
Requires: %{name} = %{version}

%description l10n
Localization files for flviewer and some its plugins.

%package libtiff
Summary: TIFF image support for %{name} using libtiff
Group: Graphics/Libraries
Requires: %{name} = %{version}
Requires: libtiff

%description libtiff
Flviewer's plugin implements viewing Tagged Image Files (TIFF).

%package poppler_pdf
Summary: PDF support for %{name} using poppler and poppler-cpp
Group: Graphics/Libraries
Requires: %{name} = %{version}
Requires: poppler-cpp poppler

%description poppler_pdf
Flviewers plugin implements viewing Portable Documents (PDF).

%if %with mupdf
%package mupdf
Summary: All formats from MuPDF
Group: Graphics/Libraries
Requires: %{name} = %{version}

%description mupdf
Flviewers plugin implements viewing all formats, supported by MuPDF (pdf, xps, cbz, epub, html/xml and some images formats).
%endif

%prep
%setup -q %{?with_mupdf:-a1}

%build
#Build MuPDF
%if %with mupdf
cd mupdf-1.8-source
export CFLAGS="-fPIC -O2 -pipe"
export LDFLAGS="-fPIC -O2 -pipe"
bash -c "make HAVE_X11=no HAVE_GLFW=no HAVE_CURL=no build=release"
cd ..
%endif

#Build flViewer
%configure %{?with_mupdf:--with-mupdf=${PWD}/mupdf-1.8-source}
make %{?_smp_mflags}

%install
%make_install
%if %with mupdf
%endif

%clean
rm -Rf %{buildroot}

%files
%doc AUTHORS ChangeLog COPYING INSTALL NEWS README.md
%{_bindir}/flviewer
%{_datadir}/applications/flviewer.desktop
%{_datadir}/pixmaps/flviewer.png
%exclude %{_libdir}/flviewer/*.la


%files plugin-devel
%doc HACKING
%{_includedir}/flviewer/*
%{_libdir}/pkgconfig/flviewer-plugin.pc

%files libtiff
%{_libdir}/flviewer/fvp_libtiff.so

%if %with mupdf
%files mupdf
%doc mupdf-1.8-source/COPYING mupdf-1.8-source/README
%{_libdir}/flviewer/fvp_mupdf.so
%endif

%files poppler_pdf
%{_libdir}/flviewer/fvp_poppler_pdf.so

%files l10n
%doc ABOUT-NLS
%{_datadir}/locale/*/LC_MESSAGES/*.mo

%changelog
* Tue Mar 22 2016 Anatoly L. Berenblit - 1.0.0-0
- Initial release

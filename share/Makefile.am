CLEANFILES= 
MOSTLYCLEANFILES=
EXTRA_DIST=

#---------------------------------------
# Desktop
#---------------------------------------
desktopdir = $(datadir)/applications
dist_desktop_DATA = flviewer.desktop

EXTRA_DIST+=flviewer.desktop.in
CLEANFILES+=$(desktop_DATA)

flviewer.desktop: flviewer.desktop.in
	cp -f $< $@

#---------------------------------------
# Pixmaps
#---------------------------------------
pixmapdir = $(datadir)/pixmaps
dist_pixmap_DATA=flviewer.png
